#include "stdio.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "ivcmp.h"

#define MAX_NUM_POINTS 10

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

  ResultScore1 = CompareIVC(IVCResistor1.Voltages, IVCResistor1.Currents, MAX_NUM_POINTS,
                           IVCResistor2.Voltages, IVCResistor2.Currents, 20);

  /* Same test but curves are swapped */
  ResultScore2 = CompareIVC(IVCResistor2.Voltages, IVCResistor2.Currents, 20,
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

  printf("All tests successfully passed.\n");

  return 0;
}