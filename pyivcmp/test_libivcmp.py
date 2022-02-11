# This module test libivcmp work and check correct work of binding
from __future__ import print_function
import unittest
from ivcmp import IvCurve, CompareIvc, MAX_NUM_POINTS, SetMinVarVC, GetMinVarVC, SetMinVarVCFromCurves, \
                  VOLTAGE_AMPL, CURRENT_AMPL
from ctypes import c_double
import numpy as np


class TestIVCMPMethods(unittest.TestCase):
    def test_number_one(self):
        self.IVCResistor1 = IvCurve()
        self.IVCResistor1.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(self.IVCResistor1, self.IVCResistor1)
        self.assertTrue(res < 0.05)

    def test_number_two(self):
        self.IVCOpenCircuit = IvCurve()
        self.IVCOpenCircuit.length = MAX_NUM_POINTS
        self.IVCShortCircuit = IvCurve()
        self.IVCShortCircuit.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            self.IVCOpenCircuit.voltages[i] = c_double(VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCOpenCircuit.currents[i] = c_double(0)
            self.IVCShortCircuit.voltages[i] = c_double(0)
            self.IVCShortCircuit.currents[i] = c_double(CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(self.IVCShortCircuit, self.IVCOpenCircuit)
        self.assertTrue((res - 0.99) < 0.05)

    def test_number_three(self):
        self.IVCResistor1 = IvCurve()
        self.IVCResistor1.length = MAX_NUM_POINTS
        self.IVCResistor2 = IvCurve()
        self.IVCResistor2.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor2.voltages[i] = c_double(0.47 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor2.currents[i] = c_double(0.63 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(self.IVCResistor1, self.IVCResistor2)
        self.assertTrue((res - 0.25) < 0.05)

    def test_number_four(self):
        self.IVCResistor1 = IvCurve()
        self.IVCResistor1.length = MAX_NUM_POINTS
        self.IVCCapacitor = IvCurve()
        self.IVCCapacitor.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCResistor1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCCapacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCCapacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / MAX_NUM_POINTS)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(self.IVCResistor1, self.IVCCapacitor)
        self.assertTrue((res - 0.99) < 0.05)

    def test_number_five(self):
        self.IVCResistor1 = IvCurve()
        self.IVCResistor1.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCResistor1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)

        self.IVCCapacitor = IvCurve()
        self.IVCCapacitor.length = 20
        for i in range(self.IVCCapacitor.length):
            self.IVCCapacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / self.IVCCapacitor.length)
            self.IVCCapacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / self.IVCCapacitor.length)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res1 = CompareIvc(self.IVCResistor1, self.IVCCapacitor)
        res2 = CompareIvc(self.IVCResistor1, self.IVCCapacitor)
        self.assertTrue((res1 - res2) < 0.05)
        self.assertTrue(res1 > 0)

    def test_error_message(self):
        # An error will be printed to stdout
        # and it is not easy to get it here
        print("The following error message is a part of test. Don’t care.")
        SetMinVarVC(0, 0)

    def test_get_min_var(self):
        # An error will be printed to stdout
        # and it is not easy to get it here
        test_var_v = 1.23
        test_var_c = 4.56
        SetMinVarVC(test_var_v, test_var_c)

        test_var_v_out, test_var_c_out = GetMinVarVC()

        self.assertTrue((test_var_v_out - test_var_v) < 0.000001)
        self.assertTrue((test_var_c_out - test_var_c) < 0.000001)

    def test_set_min_var_vc_from_curves(self):
        NUM_POINTS = 100
        sigma = 0.05
        curve_oc = IvCurve()
        curve_oc.length = NUM_POINTS
        curve_sc = IvCurve()
        curve_sc.length = NUM_POINTS
        noise_v = sigma * np.random.normal(size=NUM_POINTS)
        noise_i = sigma * np.random.normal(size=NUM_POINTS)

        for i in range(NUM_POINTS):
            curve_oc.voltages[i] = np.sin(2 * np.pi * i / NUM_POINTS) + noise_v[i]
            curve_oc.currents[i] = noise_i[i]

            curve_sc.voltages[i] = noise_v[i]
            curve_sc.currents[i] = np.sin(2 * np.pi * i / NUM_POINTS) + noise_i[i]

        SetMinVarVCFromCurves(curve_oc, curve_sc)
        res = CompareIvc(curve_sc, curve_sc)
        self.assertTrue((res - 0) < 0.05)


if __name__ == "__main__":
    unittest.main()
