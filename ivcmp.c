/* This module compares two iv-curves and returnes Score from 0.0 to 1.0
 */
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
 * Returns the difference vector of vector and variable
 * 
 * @param[in] a first vector
 * @param[in] b variable
 * @param[out] v resulting vector
 * @param[in] SizeArr vector length
 */
static void SubtractVar(double *a, double b, double *v, uint32_t SizeArr)
{
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    v[i] = a[i] - b;
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
 * Returns the distance between a point and a segment
 *
 * @param[in] p point
 * @param[in] a first end of a segment
 * @param[in] b second end of a segment
 * @param[in] SizeArr dimension
 *
 * @return distance
 */
static double Dist2PtSeg(double *p, double *a, double *b, uint32_t SizeArr)
{
  double *v1 = (double *)malloc(SizeArr * sizeof(double));
  double *v2 = (double *)malloc(SizeArr * sizeof(double));
  double Result;
  SubtractVec(b, a, v1, SizeArr);
  SubtractVec(p, a, v2, SizeArr);
  double SegLen2 = Dot(v1, v1, SizeArr);
  double Proj = Dot(v1, v2, SizeArr) / SegLen2;
  if (Proj > 1)
  {
    SubtractVec(p, b, v1, SizeArr);
    Result = Dot(v1, v1, SizeArr);
  }
  else if (Proj < 0)
  {
    Result = Dot(v2, v2, SizeArr);
  }
  else Result = pow(Cross(v1, v2), 2) / SegLen2;
  free(v1); 
  free(v2);

  return Result;
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
 * Returns all distances of two iv_curves
 *
 * @param[in] Curve first curve
 * @param[in] pts second curve
 * @param[in] SizeJ number of points in the curves
 *
 * @return normalized sum of distances
 */
static double DistCurvePts(double **Curve, double **pts, uint32_t SizeJ)
{
  double res = 0.0;
  uint32_t LocMinItem = 0; double LocMin;
  double *v = (double *)malloc(SizeJ * sizeof(double));
  double *PrevNode = NULL;
  double *CurNode = NULL;
  double *NextNode = NULL;
  double *pt = NULL;
  double Dist1, Dist2;
  uint32_t j;
  uint32_t i;
  double **CurveT = (double **)malloc(SizeJ * sizeof(double *));
  double **PtsT = (double **)malloc(SizeJ * sizeof(double *));
  for (i = 0; i < SizeJ; i++)
  {
    CurveT[i] = (double*)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double));
    PtsT[i] = (double*)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double));
  }
  
  Transpose(pts, PtsT, IV_CURVE_NUM_COMPONENTS, SizeJ);

  for (j = 0; j < SizeJ; j++)
  {
    LocMin = 100000;
    pt = PtsT[j];
    for (i = 0; i < SizeJ; i++)
    {
      v[i] = (Curve[0][i] - pt[0]) * (Curve[0][i] - pt[0]) + (Curve[1][i] - pt[1]) * (Curve[1][i] - pt[1]);
      if (v[i] < LocMin)
      {
        LocMinItem = i;
        LocMin = v[i];
      }
    }

    Transpose(Curve, CurveT, IV_CURVE_NUM_COMPONENTS, SizeJ);
    CurNode = CurveT[LocMinItem];
    
    if (LocMinItem > 0)
    {
      PrevNode = CurveT[LocMinItem - 1];
      Dist1 = Dist2PtSeg(pt, PrevNode, CurNode, IV_CURVE_NUM_COMPONENTS);
    }
    else
    {
      Dist1 = 10000;
    }
   
    if (LocMinItem < SizeJ - 1)
    {
      NextNode = CurveT[LocMinItem + 1];
      Dist2 = Dist2PtSeg(pt, CurNode, NextNode, IV_CURVE_NUM_COMPONENTS);
    }
    else
    {
      Dist2 = 10000;
    }
    res += min(Dist1, Dist2);
  }
  res /= SizeJ;
  CleanUp(CurveT, PtsT, v, NULL);
  return res;
}

static double Abs(double x)
{
  return x > 0 ? x : -x;
}

/**
 * Removes repeated data in curve
 *
 * @param a curve
 * @param[in] SizeJ number of points in the curve
 *
 * @return number of points in the cleaned curve
 */
static uint32_t RemoveRepeatsIvc(double **a, uint32_t SizeJ)
{
  uint32_t i;
  uint32_t n;
  n = 0;
  for (i = 0; i < SizeJ - 1; i++)
  {
    if ((Abs(a[0][i + 1] - a[0][i]) > 1.e-6) | (Abs(a[1][i + 1] - a[1][i]) > 1.e-6))
    {
      a[0][n] = a[0][i];
      a[1][n++] = a[1][i];
    }
  }
  a[0][n] = a[0][SizeJ - 1];
  a[1][n++] = a[1][SizeJ - 1];

  return n;
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
 * @param NewMinV new voltage noise
 * @param NewMinC new current noise
 */
void SetMinVC(double NewMinV, double NewMinC)
{
  if (NewMinV > 0 && NewMinC > 0)
  {
    MinVarV = NewMinV;
    MinVarC = NewMinC;
  }
  else
  {
    printf("IVCMP: Incorrect MinVarV, MinVarC setup. Got %lf, %lf. Should be > 0.\n",
           MinVarV, MinVarC);
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
void SetMinVCFromCurves(double *VoltagesOpenC, double *CurrentsOpenC, uint32_t CurveLengthOpenC,
                        double *VoltagesShortC, double *CurrentsShortC, uint32_t CurveLengthShotC)
{
  /*
   * Scaling threshold should be n * sigma.
   * Sigma - standard deviation of noise.
   * Open circuit – no current, so we can evaluate current noise.
   * Short circuit – no voltage drop, so we can evaluate voltage noise.
   */
  const float SigmaFactor = 3.0;
  MinVarV = SigmaFactor * sqrt(Disp(VoltagesShortC, CurveLengthShotC));
  MinVarC = SigmaFactor * sqrt(Disp(CurrentsOpenC, CurveLengthOpenC));

  /* To avoid warnings for unused params, but save standard interface */
  (void)(VoltagesOpenC);
  (void)(CurrentsShortC);
}


/**
 * Compares two curves
 * 
 * @param[in] VoltagesA voltages of the first curve
 * @param[in] CurrentsA currents of the first curve
 * @param[in] CurveLengthA number of points in the curves
 * @param[in] VoltagesB voltages of the second curve
 * @param[in] CurrentsB currents of the second curve
 * @param[in] CurveLengthB number of points in the curves
 * 
 * @return score of difference between the curves; 1.0 for completely different curves, 0.0 for same curves
 */
double CompareIVC(double *VoltagesA, double *CurrentsA, uint32_t CurveLengthA,
                  double *VoltagesB, double *CurrentsB, uint32_t CurveLengthB)
{
  uint32_t i;
  double VarV, VarC;
  double Score;

  /* Check parameters */
  if (CurveLengthA <= MIN_LEN_CURVE || CurveLengthB <= MIN_LEN_CURVE)
  {
    return SCORE_ERROR;
  }

  if (MinVarC <= 0 || MinVarV <= 0)
  {
    /*
     * Min variance should be at least several times larger than noise dispersion.
     * Optimal value – possible curve size.
     */
    return SCORE_ERROR;
  }

#ifdef DEBUG_FILE_OUTPUT
  FILE *DebugOutFile;
#endif

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "input_curve_a.txt", "w");
  for (i = 0; i < CurveLengthA; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", VoltagesA[i], CurrentsA[i]);
  }
  fclose(DebugOutFile);

  OPEN_FILE(DebugOutFile, "input_curve_b.txt", "w");
  for (i = 0; i < CurveLengthB; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", VoltagesB[i], CurrentsB[i]);
  }
  fclose(DebugOutFile);
#endif

  double **a_ = (double**)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double*));
  double **b_ = (double**)malloc(IV_CURVE_NUM_COMPONENTS * sizeof(double*));

  const uint32_t CurveLength = max(CurveLengthA, CurveLengthB);
  for (i = 0; i < IV_CURVE_NUM_COMPONENTS; i++)
  {
    a_[i] = (double*)malloc(CurveLength * sizeof(double));
    b_[i] = (double*)malloc(CurveLength * sizeof(double));
  }

  
  if (!VoltagesA | !CurrentsA)
  {
    CleanUp(a_, b_, NULL, NULL);
    return -1;
  }

  for (i = 0; i < CurveLengthA; i++)
  {
    a_[0][i] = VoltagesA[i];
    a_[1][i] = CurrentsA[i];
  }

  if (VoltagesB)
  {
    for (i = 0; i < CurveLengthB; i++)
    {
      b_[0][i] = VoltagesB[i];
      b_[1][i] = CurrentsB[i];
    }
  }

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "copied_curve_a.txt", "w");
  for (i = 0; i < CurveLengthA; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", a_[0][i], a_[1][i]);
  }
  fclose(DebugOutFile);

  OPEN_FILE(DebugOutFile, "copied_curve_b.txt", "w");
  for (i = 0; i < CurveLengthB; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", b_[0][i], b_[1][i]);
  }
  fclose(DebugOutFile);
#endif

  double _v = max(sqrt(Disp(a_[0], CurveLengthA)), sqrt(Disp(b_[0], CurveLengthB)));
  double _c = max(sqrt(Disp(a_[1], CurveLengthA)), sqrt(Disp(b_[1], CurveLengthB)));
  VarV = max(_v, MinVarV);
  VarC = max(_c, MinVarC);

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "variations.txt", "w");
  fprintf(DebugOutFile, "VarV = %lf\n", VarV);
  fprintf(DebugOutFile, "VarC = %lf\n", VarC);
  fclose(DebugOutFile);
#endif

  for (i = 0; i < CurveLengthA; i++)
  { 
    a_[0][i] = a_[0][i] / VarV;
    a_[1][i] = a_[1][i] / VarC;
  }

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "scaled_a.txt", "w");
  for (i = 0; i < CurveLengthA; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", a_[0][i], a_[1][i]);
  }
  fclose(DebugOutFile);
#endif

  double *InCurve = (double *)malloc((CurveLength * IV_CURVE_NUM_COMPONENTS + 1) * sizeof(double));
  double *OutCurve = (double *)malloc((CurveLength * IV_CURVE_NUM_COMPONENTS + 1) * sizeof(double));
  uint32_t SizeA = RemoveRepeatsIvc(a_, CurveLengthA);

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "repeats_removed_a.txt", "w");
  for (i = 0; i < SizeA; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", a_[0][i], a_[1][i]);
  }
  fclose(DebugOutFile);
#endif

  if (SizeA < MIN_LEN_CURVE)
  {
    Score = SCORE_ERROR;
    printf("ERROR:  all elements of curve identical. Algorithm doesn't match such curves!");
    CleanUp(a_, b_, InCurve, OutCurve);
    return Score;
  }

  for (i = 0; i < SizeA; i++)
  {
    InCurve[i * IV_CURVE_NUM_COMPONENTS + 1] = a_[0][i];
    InCurve[i * IV_CURVE_NUM_COMPONENTS + 2] = a_[1][i];
  }
  for (i = 1; i <= IV_CURVE_NUM_COMPONENTS * CurveLength; i++)
  {
    OutCurve[i] = 0.;
  }

  Bspline(SizeA, ORDER, CurveLength, InCurve, OutCurve);

  for (i = 0; i < CurveLength; i++)
  {
    a_[0][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 1];
    a_[1][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 2];
  }

#ifdef DEBUG_FILE_OUTPUT
  OPEN_FILE(DebugOutFile, "splined_a.txt", "w");
  for (i = 0; i < CurveLength; i++)
  {
      fprintf(DebugOutFile, "%lf\t%lf\n", a_[0][i], a_[1][i]);
  }
  fclose(DebugOutFile);
#endif

  if (!VoltagesB)
  {
    double x = Mean(a_[1], CurveLengthB);
    Score = RescaleScore(x * x);
  }
  else
  {
    for (i = 0; i < CurveLengthB; i++)
    {
      b_[0][i] = b_[0][i] / VarV;
      b_[1][i] = b_[1][i] / VarC;
    }

#ifdef DEBUG_FILE_OUTPUT
    OPEN_FILE(DebugOutFile, "scaled_b.txt", "w");
    for (i = 0; i < CurveLengthB; i++)
    {
        fprintf(DebugOutFile, "%lf\t%lf\n", b_[0][i], b_[1][i]);
    }
    fclose(DebugOutFile);
#endif

    uint32_t SizeB = RemoveRepeatsIvc(b_, CurveLengthB);

#ifdef DEBUG_FILE_OUTPUT
    OPEN_FILE(DebugOutFile, "repeats_removed_b.txt", "w");
    for (i = 0; i < SizeB; i++)
    {
        fprintf(DebugOutFile, "%lf\t%lf\n", b_[0][i], b_[1][i]);
    }
    fclose(DebugOutFile);
#endif

    if (SizeB < MIN_LEN_CURVE)
    {
      Score = SCORE_ERROR;
      printf("ERROR:  all elements of curve identical. Algorithm doesn't match such curves!");
      CleanUp(a_, b_, InCurve, OutCurve);
      return Score;
    }

    for (i = 0; i < SizeB; i++)
    {
      InCurve[i * IV_CURVE_NUM_COMPONENTS + 1] = b_[0][i];
      InCurve[i * IV_CURVE_NUM_COMPONENTS + 2] = b_[1][i];
    }

    for (i = 1; i <= IV_CURVE_NUM_COMPONENTS * CurveLength; i++)
    {
      OutCurve[i] = 0.;
    }

    Bspline(SizeB, ORDER, CurveLength, InCurve, OutCurve);
    for (i = 0; i < CurveLength; i++)
    {
      b_[0][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 1];
      b_[1][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 2];
    }

#ifdef DEBUG_FILE_OUTPUT
    OPEN_FILE(DebugOutFile, "splined_b.txt", "w");
    for (i = 0; i < CurveLength; i++)
    {
        fprintf(DebugOutFile, "%lf\t%lf\n", b_[0][i], b_[1][i]);
    }
    fclose(DebugOutFile);
#endif

    double DistAB = DistCurvePts(a_, b_, CurveLength);
    double DistBA = DistCurvePts(b_, a_, CurveLength);
    Score = RescaleScore((DistAB + DistBA) / 2.);

#ifdef DEBUG_FILE_OUTPUT
    OPEN_FILE(DebugOutFile, "dist_and_scores.txt", "w");
    fprintf(DebugOutFile, "dist_a_b = %lf\n", DistAB);
    fprintf(DebugOutFile, "dist_b_a = %lf\n", DistBA);
    fprintf(DebugOutFile, "score = %lf\n", Score);
    fclose(DebugOutFile);
#endif
  }
  CleanUp(a_, b_, InCurve, OutCurve);
  return Score;
}

