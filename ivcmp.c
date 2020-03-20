/* This is module compares two iv-curves and returnes Score (0-100 %)
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "ivcmp.h"

/*********************************/
/*    Definitions      */
/*********************************/
#define min(a, b) (((a < b)) ? (a) : (b))
#define max(a, b) (((a > b)) ? (a) : (b))
#define IV_CURVE_NUM_COMPONENTS 2
#define MIN_VAR_V_DEFAULT 0.6
#define MIN_VAR_C_DEFAULT 0.0002
static double MinVarV, MinVarC;

/*********************************/
/*  Internal functions     */
/*********************************/

/*
* Returns the difference vector of two vectors
*/
static void SubtractVec(double * a, double * b, double * v, uint32_t SizeArr)
{
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    v[i] = a[i] - b[i];
  }
}

/*
* Returns the difference vector of vector and variable
*/
static void SubtractVar(double * a, double b, double* v, uint32_t SizeArr)
{
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    v[i] = a[i] - b;
  }
}

/*
* Returns the vector mean
*/
static double Mean(double * mas, uint32_t SizeArr)
{
  double avg = 0;
  uint32_t i;
  for (i = 0; i < SizeArr; i++)
  {
    avg += mas[i];
  }
  return avg / SizeArr;
}

/*
* Returns the scalar product of two vectors
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

/*
* Returns the vector product of two vectors
*/
static double Cross(double *a, double *b)
{
  return a[0] * b[1] - a[1] * b[0];
}

/*
* Returns the dispersion of vector
*/
static double Disp(double * mas, uint32_t SizeArr)
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

/*
* Returns the transpoted matrix
*/
static void Transpose(double ** m, double ** m_t, uint32_t SizeI, uint32_t SizeJ)
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

/*
* Returns the distance between 3 points
*/
static double Dist2PtSeg(double * p, double * a, double * b, uint32_t SizeArr)
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
  free(v1); free(v2);
  return Result;
}

/*
* Updates Score value
*/
static double RescaleScore(double x)
{
  return 1 - exp(-8 * x);
}

/*
* Returns all distances of two iv_curves
*/
static double DistCurvePts(double ** Curve, double ** pts, uint32_t SizeJ)
{
  double res = 0.0;
  uint32_t LocMinItem; double LocMin;
  double *v = (double *)malloc(SizeJ * sizeof(double));
  double *PrevNode = NULL;
  double *CurNode = NULL;
  double *NextNode = NULL;
  double *pt = NULL;
  double Dist1, Dist2;
  uint32_t j;
  uint32_t i;
  double **CurveT = (double **)malloc(SizeJ * sizeof(double *));
  *CurveT = (double *)malloc(SizeJ * IV_CURVE_NUM_COMPONENTS * sizeof(double));
  for (i = 1; i < SizeJ; i++)
  {
    CurveT[i] = *CurveT + i * IV_CURVE_NUM_COMPONENTS;
  }
  
  double **PtsT = (double **)malloc(SizeJ * sizeof(double *));
  *PtsT = (double *)malloc(SizeJ * IV_CURVE_NUM_COMPONENTS * sizeof(double));
  for (i = 1; i < SizeJ; i++)
  {
    PtsT[i] = *PtsT + i * IV_CURVE_NUM_COMPONENTS;
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
    PrevNode = CurveT[LocMinItem - 1];
    CurNode = CurveT[LocMinItem];
    NextNode = CurveT[LocMinItem + 1];
    
    if (LocMinItem > 0)
    {
      Dist1 = Dist2PtSeg(pt, PrevNode, CurNode, IV_CURVE_NUM_COMPONENTS);
    }
    else
    {
      Dist1 = INFINITY;
    }

    if (LocMinItem < SizeJ - 1)
    {
      Dist2 = Dist2PtSeg(pt, CurNode, NextNode, IV_CURVE_NUM_COMPONENTS);
    }
    else
    {
      Dist2 = INFINITY;
    }
    res += min(Dist1, Dist2);
  }
  res /= SizeJ;
  free(*CurveT); free(*PtsT);
  free(CurveT); free(PtsT);
  free(v);
  return res;
}

/*
* Removes repeates data in curve
*/
static int RemoveRepeatsIvc(double ** a, double eps, uint32_t SizeJ)
{
  uint32_t i;
  uint32_t j;
  uint32_t k;
  uint32_t p;
  uint32_t n;
  long double Diff;
  uint32_t SizeN = SizeJ;
  for (p = 0; p < IV_CURVE_NUM_COMPONENTS; p++)
  {
    n = SizeJ;
    for (i = 0; i <= n - 1; i++)
    {
      for (j = i + 1; j <= n - 1; j++)
      {
        Diff = abs(a[p][i] - a[p][j]);
        if (Diff < 1.e-10 * min(abs(a[p][i]), abs(a[p][j])))
        {
          for (k = j; k <= n - 1; k++)
          {
            if (k + 1 < n)
            {
              a[p][k] = a[p][k + 1];
            }
            else a[p][k] = NAN;
          }
          n = n - 1;
          j = j - 1;
        }
      }
    }
    SizeN = min(SizeN, n);
  }
  return SizeN;
}

/*
* Subroutine to generate a B-spline open knot vector with multiplicity
  equal to the order at the ends.
  
  c      = order of the basis function
  n      = the number of defining polygon vertices
  Nplus2     = index of x() for the first occurence of the maximum knot vector value
  NplusC     = maximum value of the knot vector -- $n + c$
  x()      = array containing the knot vector
*/
static void Knot(uint32_t n, int c, double *x)
{
  int NplusC, Nplus2, i;
  NplusC = n + c;
  Nplus2 = n + 2;
  x[1] = 0;
  for (i = 2; i <= NplusC; i++)
  {
    if ((i > c) && (i < Nplus2))
    {
      x[i] = x[i - 1] + 1;
    }
    else x[i] = x[i - 1];
  }
}

/*
* Subroutine to generate B-spline basis functions for open knot vectors
c    = order of the B-spline basis function
d    = first term of the basis function recursion relation
e    = second term of the basis function recursion relation
Npts   = number of defining polygon vertices
n[]    = array containing the basis functions
n[1] contains the basis function associated with B1 etc.
NplusC   = constant -- Npts + c -- maximum number of knot values
t    = parameter value
Temp[]   = Temporary array
x[]    = knot vector
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

/*  
* Subroutine to generate a B-spline curve using an uniform open knot vector
b[]    = array containing the defining polygon vertices
b[1] contains the x-component of the vertex
b[2] contains the y-component of the vertex
b[3] contains the z-component of the vertex
k       = order of the \bsp basis function
NBasis    = array containing the basis functions for a single value of t
NplusC    = number of knot values
Npts    = number of defining polygon vertices
p[,]    = array containing the curve points
p[1] contains the x-component of the point
p[2] contains the y-component of the point
p[3] contains the z-component of the point
p1      = number of points to be calculated on the curve
t       = parameter value 0 <= t <= 1
x[]     = array containing the knot vector
*/
static void Bspline(uint32_t Npts, uint32_t k, uint32_t p1, double *b, double *p)
{
  uint32_t i, j, Icount, Jcount;
  uint32_t i1;
  double *x = (double *)malloc(IV_CURVE_NUM_COMPONENTS * Npts * sizeof(double));
  uint32_t NplusC;
  double Step;
  double t;
  double *NBasis = (double *)malloc(IV_CURVE_NUM_COMPONENTS * Npts * sizeof(double));
  double Temp;
  NplusC = Npts + k;

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

  t = 0;
  Step = (x[NplusC]) / (p1 - 1);
  for (i1 = 1; i1 <= p1; i1++)
  {
    if ((double)x[NplusC] - t < 5e-40)
    {
      t = (double)x[NplusC];
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

/*********************************/
/*    Public functions     */
/*********************************/

void SetMinVC(double NewMinV, double NewMinC)
{
  if (NewMinV > 0 && NewMinC > 0)
  {
    MinVarV = NewMinV;
    MinVarC = NewMinC;
  }
  else
  {
    MinVarV = MIN_VAR_V_DEFAULT;
    MinVarC = MIN_VAR_C_DEFAULT;
  }
}

double CompareIVC(double *VoltagesA, double *CurrentsA, 
      double *VoltagesB, double *CurrentsB, 
      uint32_t  CurveLength)
{

  uint32_t i;
  double *a_[IV_CURVE_NUM_COMPONENTS];
  double *b_[IV_CURVE_NUM_COMPONENTS];
  for (i = 0; i < IV_CURVE_NUM_COMPONENTS; i++)
  {
    a_[i] = (double*)malloc(CurveLength * sizeof(double));
    b_[i] = (double*)malloc(CurveLength * sizeof(double));
  }
  
  if (!VoltagesA | !CurrentsA)
  {
    return -1;
  }
  for (i = 0; i < CurveLength; i++)
  {
    a_[0][i] = VoltagesA[i];
    a_[1][i] = CurrentsA[i];
  }
  if (VoltagesB)
  {
    for (i = 0; i < CurveLength; i++)
    {
      b_[0][i] = VoltagesB[i];
      b_[1][i] = CurrentsB[i];
    }
  }
  double VarV, VarC;
  double Score;
  double _v = max(sqrt(Disp(a_[0], CurveLength)), sqrt(Disp(b_[0], CurveLength)));
  double _c = max(sqrt(Disp(a_[1], CurveLength)), sqrt(Disp(b_[1], CurveLength)));
  VarV = max(_v, MinVarV);
  VarC = max(_c, MinVarC);
  SubtractVar(a_[0], Mean(a_[0], CurveLength), a_[0], CurveLength);
  SubtractVar(a_[1], Mean(a_[1], CurveLength), a_[1], CurveLength);
  for (i = 0; i < CurveLength; i++)
  { 
    a_[0][i] = a_[0][i] / VarV;
    a_[1][i] = a_[1][i] / VarC;

  }

  uint32_t SizeA = RemoveRepeatsIvc(a_, 0.002, CurveLength);
  for (i = 0; i < SizeA; i++)
  {
  }

  double *InCurve = (double *)malloc((CurveLength * IV_CURVE_NUM_COMPONENTS + 1) * sizeof(double));
  double *OutCurve = (double *)malloc((CurveLength * IV_CURVE_NUM_COMPONENTS + 1) * sizeof(double));
  for (i = 0; i < SizeA; i++)
  {
    InCurve[i * IV_CURVE_NUM_COMPONENTS + 1] = a_[0][i];
    InCurve[i * IV_CURVE_NUM_COMPONENTS + 2] = a_[1][i];

  }
  for (i = 1; i <= IV_CURVE_NUM_COMPONENTS * SizeA; i++)
  {
    OutCurve[i] = 0.;
  }
  Bspline(SizeA, IV_CURVE_NUM_COMPONENTS, CurveLength, InCurve, OutCurve);
  for (i = 0; i < SizeA; i++)
  {
    a_[0][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 1];
    a_[1][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 2];
  }
  
  if (!VoltagesB)
  {
    double x = Mean(a_[1], SizeA);
    Score = RescaleScore(x * x);
  }
  else
  {
    SubtractVar(b_[0], Mean(b_[0], CurveLength), b_[0], CurveLength);
    SubtractVar(b_[1], Mean(b_[1], CurveLength), b_[1], CurveLength);
    for (i = 0; i < CurveLength; i++)
    {
      b_[0][i] = b_[0][i] / VarV;
      b_[1][i] = b_[1][i] / VarC;
    }
    uint32_t SizeB = RemoveRepeatsIvc(b_, 0.002, CurveLength);

    for (i = 0; i < SizeB; i++)
    {
      InCurve[i * IV_CURVE_NUM_COMPONENTS + 1] = b_[0][i];
      InCurve[i * IV_CURVE_NUM_COMPONENTS + 2] = b_[1][i];
    }

    for (i = 1; i <= IV_CURVE_NUM_COMPONENTS * SizeB; i++)
    {
      OutCurve[i] = 0.;
    }

    Bspline(SizeB, IV_CURVE_NUM_COMPONENTS, CurveLength, InCurve, OutCurve);

    for (i = 0; i < SizeB; i++)
    {
      b_[0][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 1];
      b_[1][i] = OutCurve[i * IV_CURVE_NUM_COMPONENTS + 2];
    }
    Score = RescaleScore((DistCurvePts(a_, b_, SizeB) + DistCurvePts(b_, a_, SizeB)) / 2.);
  }
  free(InCurve);
  free(OutCurve);
  free(*b_);
  free(*a_);

  return Score;
}
