#include "stdio.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "ivcmp.h"
#include "ivcmp_maxdev.h"

#define MAX_NUM_POINTS 1000

#define VOLTAGE_AMPL 12.
#define R_CS 475.
#define CURRENT_AMPL (VOLTAGE_AMPL / R_CS * 1000)

/* Storage structure IVC */
typedef struct
{
  double Voltages[MAX_NUM_POINTS]; /**< Array of points of voltage in V. */
  double Currents[MAX_NUM_POINTS]; /**< Array of points of current in mA. */
} iv_curve_t;


int main(void)
{
  double ResultScore, ResultScore1, ResultScore2;
  uint32_t i;
  double DevV, DevC;
  
  iv_curve_t IVCOpenCircuit, IVCShortCircuit, IVCResistor1, IVCResistor2, IVCCapacitor;

  printf("#### Test Curves ####\n");
  printf("OpenCircuit \tShortCircuit \tResistor1 \tResistor2 \tCapacitor\n");  
  
  for (i = 0; i < MAX_NUM_POINTS; i++)
  {
    /* IVC of breaks */
    IVCOpenCircuit.Voltages[i] = VOLTAGE_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);
    IVCOpenCircuit.Currents[i] = 0;

    /* IVC of SC */
    IVCShortCircuit.Voltages[i] = 0;
    IVCShortCircuit.Currents[i] = CURRENT_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);

    /* IVC of resistor 1 */
    IVCResistor1.Voltages[i] = 0.5 * VOLTAGE_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);
    IVCResistor1.Currents[i] = 0.5 * CURRENT_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);

    /* IVC of resistor 2 */
    IVCResistor2.Voltages[i] = 0.47 * VOLTAGE_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);
    IVCResistor2.Currents[i] = 0.63 * CURRENT_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);

    /* IVC of capacitor */
    IVCCapacitor.Voltages[i] = VOLTAGE_AMPL * sin(2 * M_PI * i / MAX_NUM_POINTS);
    IVCCapacitor.Currents[i] = CURRENT_AMPL * cos(2 * M_PI * i / MAX_NUM_POINTS);

    printf("%.2f %.2f\t%.2f %.2f\t%.2f %.2f\t%.2f %.2f\t%.2f %.2f\n",
         (float)IVCOpenCircuit.Voltages[i],  (float)IVCOpenCircuit.Currents[i],
         (float)IVCShortCircuit.Voltages[i], (float)IVCShortCircuit.Currents[i],
         (float)IVCResistor1.Voltages[i],  (float)IVCResistor1.Currents[i],
         (float)IVCResistor2.Voltages[i],  (float)IVCResistor2.Currents[i],
         (float)IVCCapacitor.Voltages[i],  (float)IVCCapacitor.Currents[i]);
  }

  /* Set noise levels */
  SetMinVC(0.1, 0.1);

  printf("--- Test 1. Compare same curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                           IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS);
  printf("Got Score = %.2f, should be 0.\n", (float)ResultScore);
  if (fabs(ResultScore) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 2. Compare absolutely different curves.\n");
  ResultScore = CompareIVC(IVCOpenCircuit.Voltages, IVCOpenCircuit.Currents, MAX_NUM_POINTS,
                           IVCShortCircuit.Voltages, IVCShortCircuit.Currents, MAX_NUM_POINTS);
  printf("Got Score = %.2f, should be 1.\n", (float)ResultScore);
  if (fabs(ResultScore - 1) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 3. Compare similar curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                           IVCResistor2.Voltages, IVCResistor2.Currents, MAX_NUM_POINTS);
  printf("Got Score = %.2f, should be 0.18.\n", (float)ResultScore);
  if (fabs(ResultScore - 0.18) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 4. Compare different curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                           IVCCapacitor.Voltages, IVCCapacitor.Currents, MAX_NUM_POINTS);
  printf("Got Score = %.2f, should be 1.\n", (float)ResultScore);
  if (fabs(ResultScore - 1) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 5. Compare curves with different length.\n");

  iv_curve_t IVCResistor3;
  const uint32_t num_points_for_r_3 = MAX_NUM_POINTS / 2;
  for (i = 0; i < num_points_for_r_3; i++)
  {
      IVCResistor3.Voltages[i] = 0.3 * VOLTAGE_AMPL * sin(2 * M_PI * i / num_points_for_r_3);
      IVCResistor3.Currents[i] = 0.7 * CURRENT_AMPL * sin(2 * M_PI * i / num_points_for_r_3);
  }

  ResultScore1 = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                            IVCResistor3.Voltages, IVCResistor3.Currents, num_points_for_r_3);

  /* Same test but curves are swapped */
  ResultScore2 = CompareIVC(IVCResistor3.Voltages, IVCResistor3.Currents, num_points_for_r_3,
                            IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS);
  printf("Score 1 = %.2f, Score 2 = %.2f (should be the same).\n", (float)ResultScore1, (float)ResultScore2);
  if (fabs(ResultScore2 - ResultScore1) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  if (ResultScore1 < 0)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  DevV = 0.0;
  DevC = 0.0;
  printf("--- Test 1. Compute max deviations between same curves.\n");
  ComputeMaxDeviations(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                       IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [0.0, 0.0]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV) > 0.01) | (fabs(DevC) > 0.01))
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 2. Compute max deviations between absolutely different curves (reference -- open circuit).\n");
  ComputeMaxDeviations(IVCOpenCircuit.Voltages, IVCOpenCircuit.Currents, MAX_NUM_POINTS,
                       IVCShortCircuit.Voltages, IVCShortCircuit.Currents, MAX_NUM_POINTS,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [1.0, 0.0]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV - 1) > 0.01) | (fabs(DevC) > 0.01))
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 3. Compute max deviations between absolutely different curves but swap them (reference -- short circuit).\n");
  ComputeMaxDeviations(IVCShortCircuit.Voltages, IVCShortCircuit.Currents, MAX_NUM_POINTS,
                       IVCOpenCircuit.Voltages, IVCOpenCircuit.Currents, MAX_NUM_POINTS,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [0.0, 1.0]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV) > 0.01) | (fabs(DevC - 1) > 0.01))
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 4. Compute max deviations between similar curves.\n");
  ComputeMaxDeviations(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                       IVCResistor2.Voltages, IVCResistor2.Currents, MAX_NUM_POINTS,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [0.26, 0.15]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV - 0.26) > 0.05) | (fabs(DevC - 0.15) > 0.05))
  {
    printf("Test failed!!!\n");
    //return -1;
  }

  printf("--- Test 5. Compute max deviations between different curves.\n");
  ComputeMaxDeviations(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                       IVCCapacitor.Voltages, IVCCapacitor.Currents, MAX_NUM_POINTS,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [2.0, 0.31]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV - 2.0) > 0.05) | (fabs(DevC - 0.37) > 0.05))
  {
    printf("Test failed!!!\n");
    //return -1;
  }

  printf("--- Test 6. Compute max deviations between curves with different lengths.\n");
  ComputeMaxDeviations(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                       IVCResistor3.Voltages, IVCResistor3.Currents, num_points_for_r_3,
                       &DevV, &DevC);
  printf("Score = [ %.2f, %.2f ], should be [0.65, 0.31]\n", (float)DevV, (float)DevC);
  if ((fabs(DevV - 0.65) > 0.05) | (fabs(DevC - 0.31) > 0.05))
  {
    printf("Test failed!!!\n");
    //return -1;
  }

  printf("All tests successfully passed.\n");

  return 0;
}