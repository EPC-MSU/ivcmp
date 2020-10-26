/* This module computes maximum deviation between reference and test curve
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "ivcmp_maxdev.h"

#define SCORE_ERROR -1
#define MIN_NORM_V 0.6
#define MIN_NORM_C 0.0002
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

/** Returns maximum distance between two curves
 * 
 * @param[in] Curve reference curve
 * @param[in] SizeCurve number of points in the reference curve
 * @param[in] pts test curve
 * @param[in] Sizepts number of ponts in test curve
 * 
 * @return maximum distance between two nearby points of the curves
 */
static double DistCurvePts(double *Curve, uint32_t SizeCurve, double *pts, uint32_t Sizepts)
{
    uint32_t i, j;
    double dist;
    double DistMin;
    double DistMax = 0;

    for (i = 0; i < SizeCurve; i++)
    {
        DistMin = 100000;
        for (j = 0; j < Sizepts; j++)
        {
            dist = Abs(Curve[i] - pts[j]);
            if (dist < DistMin)
            {
                DistMin = dist;
            }
        }
        if (DistMin > DistMax)
        {
            DistMax = DistMin;
        }
    }

    return DistMax;
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

    for (i=0; i < CurveLengthRef; i++)
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

double ComputeMaxDeviations(double *VoltagesRef, double *CurrentsRef, uint32_t CurveLengthRef,
                          double *VoltagesTest, double *CurrentsTest, uint32_t CurveLengthTest,
                          double *ScoreV, double *ScoreC)
{
    double normV = MIN_NORM_V;
    double normC = MIN_NORM_C;


    if (!VoltagesRef | !CurrentsRef)
    {
        *ScoreV = SCORE_ERROR;
        *ScoreC = SCORE_ERROR;
        return -1;
    }
    
    printf("%f, %f \n", (float)normV, (float)normC);
    SetNorm(VoltagesRef, &normV, CurrentsRef, &normC, CurveLengthRef);
    printf("%f, %f \n", (float)normV, (float)normC);

    //CurveLength should be same for Voltages and Currents of the same curve (???)
    uint32_t SizeRef = RemoveRepeats(VoltagesRef, CurrentsRef, CurveLengthRef);
    printf("Removed repeats, SizeRef %lu \n", (unsigned long)SizeRef);
    uint32_t SizeTest = RemoveRepeats(VoltagesTest, CurrentsTest, CurveLengthTest);
    printf("Removed repeats, SizeTest %lu \n", (unsigned long)SizeTest);

    if ((SizeRef < MIN_LEN_CURVE) || (SizeTest < MIN_LEN_CURVE))
    {
        printf("ERROR: one of the curves have identical elements.");
        *ScoreV = SCORE_ERROR;
        *ScoreC = SCORE_ERROR;
        return -1;
    }

    /*BSpline();*/

    *ScoreV = DistCurvePts(VoltagesRef, SizeRef, VoltagesTest, SizeTest);
    printf("ScoreV %.2f \n", (float)*ScoreV);
    *ScoreC = DistCurvePts(CurrentsRef, SizeRef, CurrentsTest, SizeTest);
    printf("ScoreC %.2f \n", (float)*ScoreC);

    *ScoreV = RescaleDev(*ScoreV, normV);
    printf("ScoreV %.2f \n", (float)*ScoreV);
    *ScoreC = RescaleDev(*ScoreC, normC);
    printf("ScoreC %.2f \n", (float)*ScoreC);

    return 0;
}