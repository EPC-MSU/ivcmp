#include "stdio.h"
#include "stdlib.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "ivcmp.h"

#define MAX_NUM_POINTS 20

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
  
  iv_curve_t IVCOpenCircuit, IVCShortCircuit, IVCResistor1, IVCResistor2, IVCCapacitor;
  const uint32_t CurveLength = MAX_NUM_POINTS;

  printf("#### Test Curves ####\n");
  printf("OpenCircuit \tShortCircuit \tResistor1 \tResistor2 \tCapacitor\n");  
  
  for (i = 0; i < CurveLength; i++)
  {
    /* IVC of breaks */
    IVCOpenCircuit.Voltages[i] = VOLTAGE_AMPL * sin(2 * M_PI * i / CurveLength);
    IVCOpenCircuit.Currents[i] = (float)(rand() % 200 - 100) / 100;

    /* IVC of SC */
	IVCShortCircuit.Voltages[i] = (float)(rand() % 200 - 100) / 100;
    IVCShortCircuit.Currents[i] = CURRENT_AMPL * sin(2 * M_PI * i / CurveLength);

    /* IVC of resistor 1 */
    IVCResistor1.Voltages[i] = 0.5 * VOLTAGE_AMPL * sin(2 * M_PI * i / CurveLength);
    IVCResistor1.Currents[i] = 0.5 * CURRENT_AMPL * sin(2 * M_PI * i / CurveLength);

    /* IVC of resistor 2 */
    IVCResistor2.Voltages[i] = 0.47 * VOLTAGE_AMPL * sin(2 * M_PI * i / CurveLength);
    IVCResistor2.Currents[i] = 0.63 * CURRENT_AMPL * sin(2 * M_PI * i / CurveLength);

    /* IVC of capacitor */
    IVCCapacitor.Voltages[i] = VOLTAGE_AMPL * sin(2 * M_PI * i / CurveLength);
    IVCCapacitor.Currents[i] = CURRENT_AMPL * cos(2 * M_PI * i / CurveLength);

    printf("%.2f %.2f\t%.2f %.2f\t%.2f %.2f\t%.2f %.2f\t%.2f %.2f\n",
         (float)IVCOpenCircuit.Voltages[i],  (float)IVCOpenCircuit.Currents[i],
         (float)IVCShortCircuit.Voltages[i], (float)IVCShortCircuit.Currents[i],
         (float)IVCResistor1.Voltages[i],  (float)IVCResistor1.Currents[i],
         (float)IVCResistor2.Voltages[i],  (float)IVCResistor2.Currents[i],
         (float)IVCCapacitor.Voltages[i],  (float)IVCCapacitor.Currents[i]);
  }

  /* Set measurement scaling threshold */
  SetMinVC(VOLTAGE_AMPL / 30, CURRENT_AMPL / 30);

  printf("--- Test 1. Compare same curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength,
                           IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength);
  printf("Got Score = %.2f, should be 0.\n", (float)ResultScore);
  if (fabs(ResultScore) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 2. Compare absolutely different curves.\n");
  ResultScore = CompareIVC(IVCOpenCircuit.Voltages, IVCOpenCircuit.Currents, CurveLength,
                           IVCShortCircuit.Voltages, IVCShortCircuit.Currents, CurveLength);
  printf("Got Score = %.2f, should be 1.\n", (float)ResultScore);
  if (fabs(ResultScore - 1) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  /* Set measurement scaling threshold an other way */
  SetMinVCFromCurves(IVCOpenCircuit.Voltages, IVCOpenCircuit.Currents, CurveLength,
  					 IVCShortCircuit.Voltages, IVCShortCircuit.Currents, CurveLength);

  printf("--- Test 3. Compare similar curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength,
                           IVCResistor2.Voltages, IVCResistor2.Currents, CurveLength);
  printf("Got Score = %.2f, should be 0.18.\n", (float)ResultScore);
  if (fabs(ResultScore - 0.18) > 0.1)
  {
    printf("Test failed!!!\n");
    return -1;
  }

  printf("--- Test 4. Compare different curves.\n");
  ResultScore = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength,
                           IVCCapacitor.Voltages, IVCCapacitor.Currents, CurveLength);
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

  ResultScore1 = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength,
                            IVCResistor3.Voltages, IVCResistor3.Currents, num_points_for_r_3);

  /* Same test but curves are swapped */
  ResultScore2 = CompareIVC(IVCResistor3.Voltages, IVCResistor3.Currents, num_points_for_r_3,
                            IVCResistor1.Voltages, IVCResistor1.Currents, CurveLength);
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

  printf("All tests successfully passed.\n");

  return 0;
}