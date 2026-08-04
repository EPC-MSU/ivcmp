/* This module compares two iv-curves and returnes Score from 0.0 to 1.0
 */
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "ivcmp.h"

/* ******************************* */
/*    Settings                     */
/* ******************************* */
/* Uncomment this to save temporary results 
 * to files on intermediate steps
 */
//#define DEBUG_FILE_OUTPUT

/* ******************************* */
/*    Definitions                  */
/* ******************************* */
#define IV_CURVE_NUM_COMPONENTS 2
#define MIN_VAR_V_DEFAULT 0.6
#define MIN_VAR_C_DEFAULT 0.0002
static double MinVarV, MinVarC;
static double RangeV, RangeC;
#define SCORE_ERROR -1    /**< Algorithm return Error */
#define ORDER 3     /**< Order of B-spline */
#define MIN_LEN_CURVE 2

#if defined(linux)
#define min(a, b) (((a<b))?(a):(b))
#define max(a, b) (((a>b))?(a):(b))
#endif

#if defined(linux)
#define OPEN_FILE(FilePtr, FileName, Mode) file_ptr = fopen(FileName, Mode)
#else
#define OPEN_FILE(FilePtr, FileName, Mode) fopen_s(&FilePtr, FileName, Mode)
#endif


/* ******************************* */
/*       Internal functions        */
/* ******************************* */

/**
 * This function packs separate synchronized 1D arrays of voltages and currents
 * into a single pre-allocated 2D destination array representing a curve.
 *
 * @param[in] Voltages - Pointer to the source array containing voltage values.
 * @param[in] Currents - Pointer to the source array containing current values.
 * @param[in] Length - The number of elements to copy from each source array.
 * @param[out] Curve - Pointer to the destination 2D array, where:
 * - `Curve[0]` stores the copied voltage array.
 * - `Curve[1]` stores the copied current array.
 */
static void CopyCurve(double *Voltages, double *Currents, uint32_t Length, double **Curve)
{
  uint32_t i;
  for (i = 0; i < Length; i++)
  {
    Curve[0][i] = Voltages[i];
    Curve[1][i] = Currents[i];
  }
}

/**
 * This internal function normalizes or scales a 2D curve array in-place. It divides
 * all elements in Row 0 (Voltages) by `VarV` and all elements in Row 1 (Currents) by `VarC`.
 *
 * @param[in,out] Curve - Pointer to the 2D array representing the curve, where:
 * - `Curve[0]` points to the voltage array to be scaled.
 * - `Curve[1]` points to the current array to be scaled.
 * @param[in] Length - The number of data points inside the curve arrays.
 * @param[in] VarV - The scaling factor for voltages (must be non-zero).
 * @param[in] VarC - The scaling factor for currents (must be non-zero).
 */
static void ScaleCurve(double **Curve, uint32_t Length, double VarV, double VarC)
{
  uint32_t i;
  for (i = 0; i < Length; i++)
  {
    Curve[0][i] = Curve[0][i] / VarV;
    Curve[1][i] = Curve[1][i] / VarC;
  }
}

/**
 * This internal function opens the specified file in write mode ("w") and logs
 * data in two columns: Voltages followed by Currents, separated by a tab character.
 *
 * @param[in] FileName - Path to the destination file where data will be saved.
 * @param[in] Voltages - Pointer to the array containing voltage values.
 * @param[in] Currents - Pointer to the array containing current values.
 * @param[in] Length - The total number of elements to write from the arrays.
 */
static void WriteVoltagesAndCurrentsToFile(const char *FileName, double *Voltages, double *Currents, uint32_t Length)
{
  FILE *DebugOutFile = NULL;
  uint32_t i;
  if (FileName == NULL || Voltages == NULL || Currents == NULL || Length == 0)
  {
    return;
  }

  OPEN_FILE(DebugOutFile, FileName, "w");
  if (DebugOutFile == NULL)
  {
    return;
  }

  for (i = 0; i < Length; i++)
  {
    fprintf(DebugOutFile, "%lf\t%lf\n", Voltages[i], Currents[i]);
  }
  fclose(DebugOutFile);
}

/**
 * This internal function extracts synchronized voltage and current points from
 * a 2D matrix buffer and logs them into a file. Row 0 is treated as X-axis (Voltages)
 * and Row 1 is treated as Y-axis (Currents).
 *
 * @param[in] FileName - Path to the destination file where data will be saved.
 * @param[in] Curve - Pointer to the source 2D array, where:
 * - `Curve` holds the array of voltage values.
 * - `Curve` holds the array of current values.
 * @param[in] Length - The total number of data points to write from the curve.
 */
static void WriteCurveToFile(const char *FileName, double **Curve, uint32_t Length)
{
  FILE *DebugOutFile = NULL;
  uint32_t i;

  if (FileName == NULL || Curve == NULL || Curve[0] == NULL || Curve[1] == NULL || Length == 0)
  {
    return;
  }

  OPEN_FILE(DebugOutFile, FileName, "w");
  if (DebugOutFile == NULL)
  {
    return;
  }

  for (i = 0; i < Length; i++)
  {
    fprintf(DebugOutFile, "%lf\t%lf\n", Curve[0][i], Curve[1][i]);
  }
  fclose(DebugOutFile);
}

/**
 * Returns the difference vector of two vectors
 * 
 * @param[in] a first vector
 * @param[in] b vector to subtract
 * @param[out] v resulting vector
 * @param[in] SizeArr vector length
 */
static void SubtractVec(double *a, double *b, double *v, uint32_t SizeArr)
{
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    v[i] = a[i] - b[i];
  }
}

/**
 * Returns the vector mean
 *
 * @param[in] mas vector
 * @param[in] SizeArr vector length
 *
 * @return mean of 'mas'
 */
static double Mean(double *mas, uint32_t SizeArr)
{
  double avg = 0;
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    avg += mas[i];
  }
  return avg / SizeArr;
}

/**
 * Returns the scalar product of two vectors
 *
 * @param[in] a first vector
 * @param[in] b second vector
 * @param[in] SizeArr vector length
 *
 * @return scalar product of 'a' and 'b'
 */
static double Dot(double *a, double *b, uint32_t SizeArr)
{
  double s;
  double sum = 0;
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    s = a[i] * b[i];
    sum += s;
  }
  return sum;
}

/**
 * Returns the vector product of two vectors
 *
 * @param[in] a first vector
 * @param[in] b second vector
 * @param[in] SizeArr vector length
 *
 * @return vector product of 'a' and 'b'
 */
static double Cross(double *a, double *b)
{
  return a[0] * b[1] - a[1] * b[0];
}

/*
 * Clean two double matrixes and two or less dynamic massives
 *
 * @param[in] Matrix1 first double matrix
 * @param[in] Matrix2 second double matrix
 * @param[in] Massive1 first double array
 * @param[in] Massive2 second double array
 */
static void CleanUp(double **Matrix1, double **Matrix2, double *Massive1, double *Massive2)
{
  uint32_t i;
  for (i = 0; i < IV_CURVE_NUM_COMPONENTS; i++)
  {
    free(Matrix1[i]);
    free(Matrix2[i]);
  }
  free(Matrix1);
  free(Matrix2);

  if (Massive1 != NULL)
  {
    free(Massive1);
  }

  if (Massive2 != NULL)
  {
    free(Massive2);
  }
}

/**
 * Returns the dispersion of the vector
 *
 * @param[im] mas vector
 * @param[in] SizeArr vector length
 *
 * @return ('mas' - mean of the 'mas') ^ 2
 */
static double Disp(double *mas, uint32_t SizeArr)
{
  double avg = 0;
  uint32_t i;
  double *Disp = (double *)malloc(SizeArr * sizeof(double));
  avg = Mean(mas, SizeArr);

  for (i = 0; i < SizeArr; i++)
  {
    Disp[i] = (mas[i] - avg) * (mas[i] - avg);
  }

  avg = Mean(Disp, SizeArr);
  free(Disp);
  return avg;
}

/**
 * Returns the transposed matrix
 *
 * @param[in] m matrix
 * @param[out] m_t transposed matrix
 * @param[in] Size_I number of lines in 'm'
 * @param[in] SIze_J number of columns in 'm'
 */
static void Transpose(double **m, double **m_t, uint32_t SizeI, uint32_t SizeJ)
{
  uint32_t i;
  uint32_t j;
  for (i = 0; i < SizeI; i++)
  {
    for (j = 0; j < SizeJ; j++)
    {
      m_t[j][i] = m[i][j];
    }
  }
}

/**
 * This function projects the target point onto the line defined by the segment.
 * It determines whether the orthogonal projection falls outside the segment bounds
 * (returning the distance to the closest endpoint) or inside the segment bounds
 * (returning the perpendicular distance computed via the cross product).
 * All operations are optimized to utilize fixed stack arrays, eliminating dynamic memory allocations.
 *
 * @param[in] Point - Pointer to the 2D coordinate array of the target point.
 * @param[in] StartSegment - Pointer to the 2D coordinate array of the segment's starting endpoint.
 * @param[in] EndSegment - Pointer to the 2D coordinate array of the segment's terminating endpoint.
 *
 * @return The minimum Euclidean geometric distance between the point and the segment.
 */
static double CalculateDistanceFromPointToSegment(double *Point, double *StartSegment, double *EndSegment)
{
  double v1[IV_CURVE_NUM_COMPONENTS];
  double v2[IV_CURVE_NUM_COMPONENTS];
  SubtractVec(EndSegment, StartSegment, v1, IV_CURVE_NUM_COMPONENTS);
  SubtractVec(Point, StartSegment, v2, IV_CURVE_NUM_COMPONENTS);
  
  uint32_t i;
  double SegLen2 = Dot(v1, v1, IV_CURVE_NUM_COMPONENTS);
  if (SegLen2 <= 1e-12)
  {
    double DistanceSquare = 0.0;
    for (i = 0; i < IV_CURVE_NUM_COMPONENTS; i++)
    {
      DistanceSquare += v2[i] * v2[i];
    }
    return sqrt(DistanceSquare);
  }

  double DotV1V2 = Dot(v1, v2, IV_CURVE_NUM_COMPONENTS);
  double Proj = DotV1V2 / SegLen2;
  double DistanceSquare;
  if (Proj > 1)
  {
    SubtractVec(Point, EndSegment, v1, IV_CURVE_NUM_COMPONENTS);
    DistanceSquare = Dot(v1, v1, IV_CURVE_NUM_COMPONENTS);
  }
  else if (Proj < 0)
  {
    DistanceSquare = Dot(v2, v2, IV_CURVE_NUM_COMPONENTS);
  }
  else
  {
    double CrossProduct = Cross(v1, v2);
    DistanceSquare = (CrossProduct * CrossProduct) / SegLen2;
  }

  return sqrt(DistanceSquare);
}

/**
 * Updates Score value
 *
 * @param[in] x average sum of distances between two curves
 *
 * @return score 
 */
static double RescaleScore(double x)
{
  return 1 - exp(-8 * x);
}

/**
* This internal function projects each point from Curve A onto the continuous piecewise-linear
* segments defined by Curve B. It dynamically supports independent node counts for both curves.
* The function returns the maximum value among all calculated minimum point-to-segment distances.
*
* @param[in] CurveA - Pointer to the first 2D curve matrix (source points).
* @param[in] CurveLengthA - Total number of data nodes inside Curve A.
* @param[in] CurveB - Pointer to the second 2D curve matrix (target segments).
* @param[in] CurveLengthB - Total number of data nodes inside Curve B.
*
* @return The maximum directed geometric distance from Curve A to Curve B, or -1.0 if memory allocation fails.
*/
static double CalculateDistanceBetweenCurves(double **CurveA, uint32_t CurveLengthA, double **CurveB, uint32_t CurveLengthB)
{
  double Distance1, Distance2;
  double ResultDistance = 0.0;
  uint32_t i, j;
  uint32_t LocalMinItem = 0;

  double **CurveAT = (double **)calloc(CurveLengthA, sizeof(double *));
  double **CurveBT = (double **)calloc(CurveLengthB, sizeof(double *));
  if (CurveAT == NULL || CurveBT == NULL)
  {
    free(CurveAT);
    free(CurveBT);
    return -1.0;
  }

  for (i = 0; i < CurveLengthA; i++)
  {
    CurveAT[i] = (double *)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double));
    if (CurveAT[i] == NULL)
    {
      for (j = 0; j <= i; j++)
      {
        if (CurveAT[j]) free(CurveAT[j]);
      }
      free(CurveAT);
      free(CurveBT);
      return -1.0;
    }
  }

  for (i = 0; i < CurveLengthB; i++)
  {
    CurveBT[i] = (double *)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double));
    if (CurveBT[i] == NULL)
    {
      for (j = 0; j <= i; j++)
      {
        if (CurveBT[j]) free(CurveBT[j]);
      }

      for (j = 0; j < CurveLengthA; j++)
      {
        if (CurveAT[j]) free(CurveAT[j]);
      }

      free(CurveAT);
      free(CurveBT);
      return -1.0;
    }
  }

  Transpose(CurveA, CurveAT, IV_CURVE_NUM_COMPONENTS, CurveLengthA);
  Transpose(CurveB, CurveBT, IV_CURVE_NUM_COMPONENTS, CurveLengthB);

  for (j = 0; j < CurveLengthA; j++)
  {
    double LocalMin = DBL_MAX;
    double *PointA = CurveAT[j];

    for (i = 0; i < CurveLengthB; i++)
    {
      double *PointB = CurveBT[i];
      double Distance = sqrt((PointB[0] - PointA[0]) * (PointB[0] - PointA[0]) + (PointB[1] - PointA[1]) * (PointB[1] - PointA[1]));
      if (Distance < LocalMin)
      {
        LocalMin = Distance;
        LocalMinItem = i;
      }
    }
    
    if (LocalMinItem > 0)
    {
      double *PrevNode = CurveBT[LocalMinItem - 1];
      double *Node = CurveBT[LocalMinItem];
      Distance1 = CalculateDistanceFromPointToSegment(PointA, PrevNode, Node);
    }
    else
    {
      Distance1 = DBL_MAX;
    }
   
    if (LocalMinItem < CurveLengthB - 1)
    {
      double *Node = CurveBT[LocalMinItem];
      double *NextNode = CurveBT[LocalMinItem + 1];
      Distance2 = CalculateDistanceFromPointToSegment(PointA, Node, NextNode);
    }
    else
    {
      Distance2 = DBL_MAX;
    }
    
    double DistanceToCurve = min(Distance1, Distance2);
    if (ResultDistance < DistanceToCurve)
    {
      ResultDistance = DistanceToCurve;
    }
  }

  for (i = 0; i < CurveLengthA; i++)
  {
    free(CurveAT[i]);
  }

  for (i = 0; i < CurveLengthB; i++)
  {
    free(CurveBT[i]);
  }

  free(CurveAT);
  free(CurveBT);
  return ResultDistance;
}

static double Abs(double x)
{
  return x > 0 ? x : -x;
}

/**
 * This function filters out adjacent data points whose voltage and current changes
 * fall below a 1e-6 threshold. To prevent the loss of slow, continuous trends
 * (signal drift), each point is compared against the last officially saved
 * unique point (index 'n'), rather than its immediate predecessor (index 'i').
 *
 * @param[in,out] Curve - Pointer to the 2D curve matrix, where:
 * - `a[0]` represents the array of voltages.
 * - `a[1]` represents the array of currents.
 * @param[in] Size - The original number of data points inside the curve.
 *
 * @return The new length of the filtered curve (total number of unique nodes).
 */
static uint32_t RemoveRepeatsIvc(double **Curve, uint32_t Size)
{
  uint32_t i;
  uint32_t n = 0;
  for (i = 0; i < Size - 1; i++)
  {
    if ((fabs(Curve[0][i + 1] - Curve[0][n]) > 1.e-6) || (fabs(Curve[1][i + 1] - Curve[1][n]) > 1.e-6))
    {
      n++;
      Curve[0][n] = Curve[0][i + 1];
      Curve[1][n] = Curve[1][i + 1];
    }
  }

  return n + 1;
}

/**
 * Subroutine to generate a B-spline knot vector
 * 
 * @param[in] n number of defining polygon vertices
 * @param[in] c order of the basis function
 * @param[out] x knot vector
 * 
 * @note NplusC is the maximum value of the knot vector 'x'
 */
static void Knot(uint32_t n, uint32_t c, double *x)
{
  uint32_t NplusC, i;
  NplusC = n + c;
  x[1] = 0;

  for (i = 2; i <= NplusC; i++)
  {
    x[i] = i - 1;
  }
}

/**
 * Subroutine to generate B-spline basis functions for knot vectors. Uses Cox-de Boor recursive relation.
 *
 * @param[in] c order of the B-spline basis function
 * @param[in] t parameter in the Cox-de Boor formula
 * @param[in] Npts number of defining polygon vertices
 * @param[in] x knot vector
 * @param[out] n array containing the basis functions
 *
 * @note d is the first part of the basis function recursive relation
 * @note e is the second part of the basis function recursive relation
 * @note NplusC the maximum number of knot values 'Npts' + 'c'
 */
static void Basis(uint32_t c, double t, uint32_t Npts, double *x, double *n)
{
  uint32_t NplusC;
  uint32_t k; uint32_t i;
  double d, e;
  double *Temp = (double *)malloc(4 * Npts * sizeof(double));
  NplusC = Npts + c;

  for (i = 1; i <= NplusC - 1; i++)
  {
    if ((t >= x[i]) && (t < x[i + 1]))
    {
      Temp[i] = 1.000;
    }
    else Temp[i] = 0;
  }
  for (k = 2; k <= c; k++)
  {
    for (i = 1; i <= NplusC - k; i++)
    {
      if (Temp[i] != 0)
      {
        d = ((t - x[i]) * Temp[i]) / (x[i + k - 1] - x[i]);
      }
      else d = 0;

      if (Temp[i + 1] != 0)
      {
        e = ((x[i + k] - t) * Temp[i + 1]) / (x[i + k] - x[i + 1]);
      }
      else e = 0;

      Temp[i] = d + e;
    }
  }

  if (t == x[NplusC])
  {
    Temp[Npts] = 1;
  }

  for (i = 0; i <= Npts; i++)
  {
    n[i] = Temp[i];
  }
  free(Temp);
}

/**
 * Subroutine to generate a B-spline curve using an uniform open knot vector
 *
 * @param[in] Npts number of defining polygon vertices
 * @param[in] k order of the B-spline basis function
 * @param[in] p1 number of points to be calculated on the curve
 * @param[in] b array containing the defining polygon vertices
 * @param[out] p array containing the curve points
 *
 * @note b[1], b[3]... contain the x-component of the vertex
 * @note b[2], b[4]... contain the y-component of the vertex
 * @note NBasis is the array containing the basis functions for a single value of t
 * @note NplusC is the number of knot values
 * @note p[1], p[3]... contain the x-component of the point
 * @note p[2], p[4]... contain the y-component of the point
 * @note t is the parameter value used in Cox-de Boor formula
 * @note x is the array containing the knot vector
 */
static void Bspline(uint32_t Npts, uint32_t k, uint32_t p1, double *b, double *p)
{
  uint32_t i, j, Icount, Jcount;
  uint32_t i1;
  uint32_t NplusC;
  double Step;
  double t;
  double *NBasis = (double *)malloc(IV_CURVE_NUM_COMPONENTS * Npts * sizeof(double));
  double Temp;
  NplusC = Npts + k;
  double *x = (double *)malloc(IV_CURVE_NUM_COMPONENTS * NplusC * sizeof(double));

  for (i = 1; i <= Npts; i++)
  {
    NBasis[i] = 0.;
  }

  for (i = 1; i <= NplusC; i++)
  {
    x[i] = 0.;
  }

  Knot(Npts, k, x);

  Icount = 0;

  t = k - 1; /* special parameter range for periodic basis functions */
  Step = ((float)(Npts - (k - 1))) / ((float)(p1 - 1));

  for (i1 = 1; i1 <= p1; i1++)
  {
    if ((float)(Npts) - t < 5e-6)
    {
      t = (float)((Npts));
    }
    Basis(k, t, Npts, x, NBasis);
    for (j = 1; j <= 2; j++)
    {
      Jcount = j;
      p[Icount + j] = 0.;
      for (i = 1; i <= Npts; i++)
      {
        Temp = NBasis[i] * b[Jcount];
        p[Icount + j] = p[Icount + j] + Temp;
        Jcount = Jcount + 2;
      }
    }
    Icount = Icount + 2;
    t = t + Step;
  }
  free(x);
  free(NBasis);
}


/* ******************************* */
/*    Public functions             */
/* ******************************* */

/**
 * Sets scaling threshold for voltages and currents
 * 
 * @param NewMinVarV new voltage noise
 * @param NewMinVarC new current noise
 */
void SetMinVarVC(double NewMinVarV, double NewMinVarC)
{
  if (NewMinVarV > 0 && NewMinVarC > 0)
  {
    MinVarV = NewMinVarV;
    MinVarC = NewMinVarC;
  }
  else
  {
    printf("IVCMP ERROR: Incorrect MinVarV, MinVarC setup. Got %lf, %lf. Should be > 0.\n"
           "CompareIVC() will not work until correct MinVar setup\n", NewMinVarV, NewMinVarC);
    /*
     * Error maximization. Compare will return -1 until correct MinVar setup.
     */
    MinVarV = 0;
    MinVarC = 0;
  }
}


/**
 * Sets scaling threshold for voltages and currents
 * by noise evaluation for short circuit and open circuit curves.
 * 
 * @param VoltagesOpenC Array of voltages for open circuit curve
 * @param CurrentsOpenC Array of currents for open circuit curve
 * @param CurveLengthOpenC Number of points in open circuit curve
 * @param VoltagesShortC Array of voltages for short circuit curve
 * @param CurrentsShortC Array of currents for short circuit curve
 * @param CurveLengthShotC Number of points in short circuit curve
 */
void SetMinVarVCFromCurves(double *VoltagesOpenC, double *CurrentsOpenC, uint32_t CurveLengthOpenC,
                           double *VoltagesShortC, double *CurrentsShortC, uint32_t CurveLengthShotC)
{
  /*
   * Scaling threshold should be n * sigma.
   * Sigma - standard deviation of noise.
   * Open circuit - no current, so we can evaluate current noise.
   * Short circuit - no voltage drop, so we can evaluate voltage noise.
   */
  const float SigmaFactor = 3.0;
  double NewMinVarV = SigmaFactor * sqrt(Disp(VoltagesShortC, CurveLengthShotC));
  double NewMinVarC = SigmaFactor * sqrt(Disp(CurrentsOpenC, CurveLengthOpenC));

  SetMinVarVC(NewMinVarV, NewMinVarC);

  /* To avoid warnings for unused params, but save standard interface */
  (void)(VoltagesOpenC);
  (void)(CurrentsShortC);
}


/**
 * Gets active scaling threshold for voltages and currents
 *
 * @param NewMinVarVPtr - variable pointer to store voltage variation.
 * @param NewMinVarCPtr - variable pointer to store current variation.
 */
void GetMinVarVC(double *NewMinVarVPtr, double *NewMinVarCPtr)
{
  *NewMinVarVPtr = MinVarV;
  *NewMinVarCPtr = MinVarC;
}


/**
 * This function updates the internal variables used for normalizing current-voltage
 * characteristics (IVC). It ensures that the provided range factors are strictly positive
 * to prevent division-by-zero errors during subsequent scaling procedures.
 *
 * @param[in] NewRangeV - The new scaling range factor for voltage values (must be > 0.0).
 * @param[in] NewRangeC - The new scaling range factor for current values (must be > 0.0).
*/
void SetRangesVC(double NewRangeV, double NewRangeC)
{
  if (NewRangeV <= 0.0 || NewRangeC <= 0.0)
  {
    printf("IVCMP ERROR: Invalid ranges for voltages and currents. You should explicitly set them positive values.\n");
    RangeV = 0.0;
    RangeC = 0.0;
    return;
  }

  RangeV = NewRangeV;
  RangeC = NewRangeC;
}


/**
 * This function evaluates the geometric difference between two synchronized IV curves of
 * potentially different lengths. It maps the voltage and current vectors into internal 2D curve
 * buffers, executes structural comparisons, and normalizes the final result into a standard
 * bound score.
 *
 * @param[in] VoltagesA - Pointer to the array containing voltage values of the first curve (Curve A).
 * @param[in] CurrentsA - Pointer to the array containing current values of the first curve (Curve A).
 * @param[in] CurveLengthA - The total number of data nodes/points in Curve A.
 * @param[in] VoltagesB - Pointer to the array containing voltage values of the second curve (Curve B).
 * @param[in] CurrentsB - Pointer to the array containing current values of the second curve (Curve B).
 * @param[in] CurveLengthB - The total number of data nodes/points in Curve B.
 *
 * @return A normalized double value indicating the degree of difference:
 * - `0.0` if the curves are identical.
 * - `1.0` if the curves are completely different or maximum divergence is reached.
 * - `-1.0` if an internal memory allocation error occurs during comparison.
 */
double CompareIVC(double *VoltagesA, double *CurrentsA, uint32_t CurveLengthA,
                  double *VoltagesB, double *CurrentsB, uint32_t CurveLengthB)
{
  uint32_t i;
  double VarV, VarC;

  /* Check parameters */
  if (CurveLengthA <= MIN_LEN_CURVE || CurveLengthB <= MIN_LEN_CURVE)
  {
    printf("IVCMP ERROR: The signature length is too small. There should be at least %d points.\n", MIN_LEN_CURVE);
    return SCORE_ERROR;
  }

  if (MinVarC <= 0 || MinVarV <= 0)
  {
    /*
     * Min variance should be at least several times larger than noise dispersion.
     * Optimal value - possible curve size.
     */
    printf("IVCMP ERROR: Invalid normalization thresholds (MinVarVC). You should explicitly set them.\n");
    return SCORE_ERROR;
  }

  if (RangeC <= 0 || RangeV <= 0)
  {
    printf("IVCMP ERROR: Invalid voltage or current ranges. Values must be positive.\n");
    return SCORE_ERROR;
  }

  if (!VoltagesA || !CurrentsA)
  {
    printf("IVCMP ERROR: Invalid voltage/current pointers provided for curve A.\n");
    return SCORE_ERROR;
  }

  if (!VoltagesB || !CurrentsB)
  {
    printf("IVCMP ERROR: Invalid voltage/current pointers provided for curve B.\n");
    return SCORE_ERROR;
  }

#ifdef DEBUG_FILE_OUTPUT
  WriteVoltagesAndCurrentsToFile("input_curve_a.txt", VoltagesA, CurrentsA, CurveLengthA);
  WriteVoltagesAndCurrentsToFile("input_curve_b.txt", VoltagesB, CurrentsB, CurveLengthB);
#endif

  double **a_ = (double**)calloc(IV_CURVE_NUM_COMPONENTS, sizeof(double*));
  double **b_ = (double**)calloc(IV_CURVE_NUM_COMPONENTS, sizeof(double*));
  if (a_ == NULL || b_ == NULL)
  {
    free(a_);
    free(b_);
    return SCORE_ERROR;
  }

  const uint32_t CurveLength = max(CurveLengthA, CurveLengthB);
  for (i = 0; i < IV_CURVE_NUM_COMPONENTS; i++)
  {
    a_[i] = (double*)malloc(CurveLength * sizeof(double));
    b_[i] = (double*)malloc(CurveLength * sizeof(double));

    if (a_[i] == NULL || b_[i] == NULL)
    {
      uint32_t j;
      for (j = 0; j < IV_CURVE_NUM_COMPONENTS; j++)
      {
        if (a_[j]) free(a_[j]);
        if (b_[j]) free(b_[j]);
      }
      free(a_);
      free(b_);
      return SCORE_ERROR;
    }
  }

  CopyCurve(VoltagesA, CurrentsA, CurveLengthA, a_);
  CopyCurve(VoltagesB, CurrentsB, CurveLengthB, b_);

#ifdef DEBUG_FILE_OUTPUT
  WriteCurveToFile("copied_curve_a.txt", a_, CurveLengthA);
  WriteCurveToFile("copied_curve_b.txt", b_, CurveLengthB);
#endif

  VarV = max(RangeV, MinVarV);
  VarC = max(RangeC, MinVarC);

#ifdef DEBUG_FILE_OUTPUT
  FILE *DebugOutFile = NULL;
  OPEN_FILE(DebugOutFile, "variations.txt", "w");
  fprintf(DebugOutFile, "VarV = %lf\n", VarV);
  fprintf(DebugOutFile, "VarC = %lf\n", VarC);
  fclose(DebugOutFile);
#endif

  ScaleCurve(a_, CurveLengthA, VarV, VarC);

#ifdef DEBUG_FILE_OUTPUT
  WriteCurveToFile("scaled_a.txt", a_, CurveLengthA);
#endif

  uint32_t NewCurveLengthA = RemoveRepeatsIvc(a_, CurveLengthA);

#ifdef DEBUG_FILE_OUTPUT
  WriteCurveToFile("repeats_removed_a.txt", a_, NewCurveLengthA);
#endif

  if (NewCurveLengthA < MIN_LEN_CURVE)
  {
    printf("IVCMP ERROR: All curve A elements are identical. The algorithm is unable to match such curves.\n");
    CleanUp(a_, b_, NULL, NULL);
    return SCORE_ERROR;
  }

  ScaleCurve(b_, CurveLengthB, VarV, VarC);

#ifdef DEBUG_FILE_OUTPUT
  WriteCurveToFile("scaled_b.txt", b_, CurveLengthB);
#endif

  uint32_t NewCurveLengthB = RemoveRepeatsIvc(b_, CurveLengthB);

#ifdef DEBUG_FILE_OUTPUT
  WriteCurveToFile("repeats_removed_b.txt", b_, NewCurveLengthB);
#endif

  if (NewCurveLengthB < MIN_LEN_CURVE)
  {
    printf("IVCMP ERROR: All curve B elements are identical. The algorithm is unable to match such curves.\n");
    CleanUp(a_, b_, NULL, NULL);
    return SCORE_ERROR;
  }

  double DistanceAB = CalculateDistanceBetweenCurves(a_, NewCurveLengthA, b_, NewCurveLengthB);
  double DistanceBA = CalculateDistanceBetweenCurves(b_, NewCurveLengthB, a_, NewCurveLengthA);
  double Score = max(DistanceAB, DistanceBA);

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "dist_and_scores.txt", "w");
  fprintf(DebugOutFile, "dist_a_b = %lf\n", DistanceAB);
  fprintf(DebugOutFile, "dist_b_a = %lf\n", DistanceBA);
  fprintf(DebugOutFile, "score = %lf\n", Score);
  fclose(DebugOutFile);
#endif

  CleanUp(a_, b_, NULL, NULL);
  return Score;
}
