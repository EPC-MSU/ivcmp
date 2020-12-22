/* This module computes maximum deviation between reference and test curve
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "ivcmp.h"

#define SCORE_ERROR -1.
#define MIN_NORM_V 0.1
#define MIN_NORM_C 0.00005
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

/*********************************/
/*    Internal  functions     */
/*********************************/

static double Abs(double x)
{
  return x > 0 ? x : -x;
}

/** Removes repeated data in a curve [V, C]
 * 
 * @param V voltages in the curve
 * @param C currents in the curve
 * @param[in] SizeJ number of points 
 */
static uint32_t RemoveRepeats(double *V, double *C, uint32_t SizeJ)
{
    uint32_t i, n;
    n = 0;
    for (i = 0; i < SizeJ - 1; i++){
        if ((Abs(V[i + 1] - V[i]) > 1.e-6) | (Abs(C[i + 1] - C[i]) > 1.e-6))
        {
            V[n] = V[i];
            C[n++] = C[i];
        }
    }
    V[n] = V[SizeJ - 1];
    C[n++] = C[SizeJ - 1];

    return n;
}

/**
 * Returns maximum element from two curves.
 * 
 * @param[in] a1 first curve
 * @param[in] a1size length of the first curve
 * @param[in] a2 second curve
 * @param[in] a2size length of the second curve
 * 
 * @param[out] maxval maximum value of two curves
 * 
 **/
static double MaxDuo(double *a1, uint32_t a1size, double *a2, uint32_t a2size)
{
    uint32_t i;
    double maxval = 0;

    for (i = 0; i < a1size; i++)
    {
        if (Abs(a1[i]) > maxval) {maxval = Abs(a1[i]);}
    }
    for (i = 0; i < a2size; i++)
    {
        if(Abs(a2[i]) > maxval) {maxval = Abs(a2[i]);}
    }

    return maxval;
}

/**
 * Calculates the voltage and current distances between a point and a segment
 * 
 * @param[in] x0,y0 coords of the point
 * @param[in] x1,y1 coords of the start of the segment
 * @param[in] x2,y2 coords of the end of the segment
 * @param distx voltage distance
 * @param disty current distance
 * 
 **/
static void Dist2PtSeg(double x0, double y0, double x1, double y1, double x2, double y2, double *distx, double *disty)
{
    double seglen2, proj;
    double a, b, c, d, det, detx, dety;

    seglen2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    proj = ((x0 - x1) * (x2 - x1) + (y0 - y1) * (y2 - y1)) / seglen2;
    if (proj > 1)
    {
        *distx = Abs(x0 - x2);
        *disty = Abs(y0 - y2);
    }
    else if (proj < 0)
    {
        *distx = Abs(x0 - x1);
        *disty = Abs(y0 - y1);
    }
    else
    {
        //solve linear equation using kramer's rule
        a = x2 - x1;
        if (a == 0)
        {
            *distx = Abs(x0 - x1);
            *disty = 0;
            return;
        }
        b = y2 - y1;
        if (b == 0)
        {
            *distx = 0;
            *disty = Abs(y0 - y1);
            return;
        }
        c = x2 * b - y2 * a;
        d = x0 * a + y0 * b;
        det = b * b + a * a;
        detx = c * b + a * d;
        dety = d * b - a * c;

        *distx = Abs(detx / det);
        *disty = Abs(dety / det);
    }
}

/** Returns maximum distance between two curves
 * 
 * @param[in] CurveV reference curve voltages
 * @param[in] CurveC reference curve currents
 * @param[in] SizeCurve number of points in the reference curve
 * @param[in] ptsV test curve voltages
 * @param[in] ptsC test curve currents
 * @param[in] Sizepts number of ponts in test curve
 * 
 * @param DistMaxV maximum voltage distance in a set of nearest points
 * @param DistMaxC maximum current distance in a set of nearest points
 */
static void DistCurvePts(double *CurveV, double *CurveC, uint32_t SizeCurve, double *ptsV, double *ptsC, uint32_t Sizepts,
                         double **DistMaxV, double **DistMaxC)
{
    uint32_t i, j;
    double dist, distV, distC;
    double DistMin, DistMinV, DistMinC;
    double maxV, maxC;
    distV = 0.0;
    distC = 0.0;
    **DistMaxV = 0.0;
    **DistMaxC = 0.0;

    maxV = MaxDuo(CurveV, SizeCurve, ptsV, Sizepts);
    maxC = MaxDuo(CurveC, SizeCurve, ptsC, Sizepts);

    for (i = 0; i < SizeCurve; i++)
    {
        DistMin = 100000;
        DistMinV = 100000;
        DistMinC = 100000;
        for (j = 0; j < Sizepts - 1; j++)
        {   

            Dist2PtSeg(CurveV[i] / maxV, CurveC[i] / maxC, ptsV[j] / maxV, ptsC[j] / maxC, ptsV[j+1] / maxV, ptsC[j+1] / maxC, &distV, &distC);
            dist = distV * distV + distC * distC;
            if (dist < DistMin)
            {
                DistMin = dist;
                DistMinV = distV;
                DistMinC = distC;
            }
        }
        if (DistMinV > **DistMaxV)
        {
            **DistMaxV = DistMinV;
        }
        if (DistMinC > **DistMaxC)
        {
            **DistMaxC = DistMinC;
        }

    }
    **DistMaxV = **DistMaxV * maxV;
    **DistMaxC = **DistMaxC * maxC;
}

/**
 * Rescales maximum deviation
 * @param Score maximum distance between two curves
 * @param[in] norm value to normalize distance by
 */
static double RescaleDev(double Score, double norm)
{
    return Score/norm;
}

/**
 * Sets norm as a maximum voltage and current in reference curve
 * 
 * @param normV voltages norm
 * @param normC currents norm
 */
static void SetNorm(double *VoltagesRef, double *normV, double *CurrentsRef, double *normC, uint32_t CurveLengthRef)
{
    uint32_t i;

    for (i = 0; i < CurveLengthRef; i++)
    {
        if (*normV < Abs(VoltagesRef[i])) {*normV = Abs(VoltagesRef[i]);}
        if (*normC < Abs(CurrentsRef[i])) {*normC = Abs(CurrentsRef[i]);}
    }
}

/*********************************/
/*    Public functions     */
/*********************************/

/** Computes maximum deviation between reference and test curve
 *
 * @param[in] VoltagesRef voltages of the reference  curve
 * @param[in] CurrentsRef currents of the reference curve
 * @param[in] VoltagesTest voltages of the test curve
 * @param[in] CurrentsTest currents of the test curve
 * @param[in] CurveLength number of points in the curves
 * @param ScoreV voltages deviation
 * @param ScoreC currents deviation
 */

void ComputeMaxDeviations(double *VoltagesRef, double *CurrentsRef, uint32_t CurveLengthRef,
                          double *VoltagesTest, double *CurrentsTest, uint32_t CurveLengthTest,
                          double *ScoreV, double *ScoreC)
{
    double normV = MIN_NORM_V;
    double normC = MIN_NORM_C;


    if (!VoltagesRef | !CurrentsRef)
    {
        printf("ERROR: reference curve is not given.");
        *ScoreV = SCORE_ERROR;
        *ScoreC = SCORE_ERROR;
        return;
    }

    SetNorm(VoltagesRef, &normV, CurrentsRef, &normC, CurveLengthRef);

    uint32_t SizeRef = RemoveRepeats(VoltagesRef, CurrentsRef, CurveLengthRef);
    uint32_t SizeTest = RemoveRepeats(VoltagesTest, CurrentsTest, CurveLengthTest);

    if ((SizeRef < MIN_LEN_CURVE) || (SizeTest < MIN_LEN_CURVE))
    {
        printf("ERROR: at least one of the curves have identical elements.");
        *ScoreV = SCORE_ERROR;
        *ScoreC = SCORE_ERROR;
        return;
    }

    DistCurvePts(VoltagesRef, CurrentsRef, SizeRef, VoltagesTest, CurrentsTest, SizeTest, &ScoreV, &ScoreC);

    *ScoreV = RescaleDev(*ScoreV, normV);
    *ScoreC = RescaleDev(*ScoreC, normC);
}