# This module test libivcmp work and check correct work of binding
from __future__ import print_function
import unittest
from pyivcmp.ivcmp import IvCurve, CompareIvc, MAX_NUM_POINTS, SetMinVarVC, GetMinVarVC, SetMinVarVCFromCurves, \
                          VOLTAGE_AMPL, CURRENT_AMPL
from ctypes import c_double
import numpy as np


class TestIVCMPMethods(unittest.TestCase):
    def test_r_and_same_r(self):
        ivc_resistor_1 = IvCurve()
        ivc_resistor_1.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            ivc_resistor_1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_resistor_1, ivc_resistor_1)
        self.assertTrue(res < 0.05)

    def test_short_and_open_circuit(self):
        ivc_open_circuit = IvCurve()
        ivc_open_circuit.length = MAX_NUM_POINTS
        ivc_short_circuit = IvCurve()
        ivc_short_circuit.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_open_circuit.voltages[i] = c_double(VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            ivc_open_circuit.currents[i] = c_double(0)
            ivc_short_circuit.voltages[i] = c_double(0)
            ivc_short_circuit.currents[i] = c_double(CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_short_circuit, ivc_open_circuit)
        self.assertTrue((res - 0.99) < 0.05)

    def test_r1_and_r2(self):
        ivc_resistor_1 = IvCurve()
        ivc_resistor_1.length = MAX_NUM_POINTS
        ivc_resistor_2 = IvCurve()
        ivc_resistor_2.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            ivc_resistor_1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            ivc_resistor_2.voltages[i] = c_double(0.47 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            ivc_resistor_2.currents[i] = c_double(0.63 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_resistor_1, ivc_resistor_2)
        self.assertTrue((res - 0.25) < 0.05)

    def test_r_and_c(self):
        ivc_resistor_1 = IvCurve()
        ivc_resistor_1.length = MAX_NUM_POINTS
        ivc_capacitor = IvCurve()
        ivc_capacitor.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_resistor_1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_capacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_capacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / MAX_NUM_POINTS)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_resistor_1, ivc_capacitor)
        self.assertTrue((res - 0.99) < 0.05)

    def test_c_and_shifted_c(self):
        """
        C and shifted C during charge - two different curves.
        """
        ivc_capacitor = IvCurve()
        ivc_capacitor.length = MAX_NUM_POINTS
        ivc_capacitor_shifted = IvCurve()
        ivc_capacitor_shifted.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_capacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_capacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_capacitor_shifted.voltages[i] = VOLTAGE_AMPL * (np.sin(2 * np.pi * i / MAX_NUM_POINTS) + 0.2)
            ivc_capacitor_shifted.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / MAX_NUM_POINTS)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_capacitor, ivc_capacitor_shifted)
        self.assertTrue(res > 0.2)

    def test_different_lengths(self):
        ivc_resistor_1 = IvCurve()
        ivc_resistor_1.length = MAX_NUM_POINTS
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_resistor_1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)

        ivc_capacitor = IvCurve()
        ivc_capacitor.length = 20
        for i in range(ivc_capacitor.length):
            ivc_capacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / ivc_capacitor.length)
            ivc_capacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / ivc_capacitor.length)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res1 = CompareIvc(ivc_resistor_1, ivc_capacitor)
        res2 = CompareIvc(ivc_capacitor, ivc_resistor_1)
        self.assertTrue((res1 - res2) < 0.05)

    def test_zero_length(self):
        ivc_resistor_1 = IvCurve()
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_resistor_1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)

        ivc_capacitor = IvCurve()
        for i in range(ivc_capacitor.length):
            ivc_capacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / ivc_capacitor.length)
            ivc_capacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / ivc_capacitor.length)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        with self.assertRaises(ValueError):
            CompareIvc(ivc_resistor_1, ivc_capacitor)

    def test_array_assignment(self):
        ivc_resistor_1 = IvCurve()
        voltages = []
        currents = []
        for i in range(MAX_NUM_POINTS):
            voltages.append(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            currents.append(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
        ivc_resistor_1.length = MAX_NUM_POINTS
        ivc_resistor_1.voltages = voltages
        ivc_resistor_1.currents = currents

        ivc_capacitor = IvCurve()
        i = np.arange(MAX_NUM_POINTS)
        ivc_capacitor.length = MAX_NUM_POINTS
        ivc_capacitor.voltages = VOLTAGE_AMPL * np.sin(2 * np.pi * i / ivc_capacitor.length)
        ivc_capacitor.currents = CURRENT_AMPL * np.cos(2 * np.pi * i / ivc_capacitor.length)

        # Set Voltage and Current scale
        SetMinVarVC(VOLTAGE_AMPL * 0.03, CURRENT_AMPL * 0.03)

        res = CompareIvc(ivc_resistor_1, ivc_capacitor)
        self.assertTrue((res - 0.99) < 0.05)

        res = CompareIvc(ivc_resistor_1, ivc_resistor_1)
        self.assertTrue(res < 0.05)

        res = CompareIvc(ivc_capacitor, ivc_capacitor)
        self.assertTrue(res < 0.05)

        i = np.arange(MAX_NUM_POINTS * 2)
        ivc_capacitor.length = MAX_NUM_POINTS
        with self.assertRaises(ValueError):
            ivc_capacitor.voltages = VOLTAGE_AMPL * np.sin(2 * np.pi * i / ivc_capacitor.length)

        with self.assertRaises(ValueError):
            ivc_capacitor.currents = CURRENT_AMPL * np.cos(2 * np.pi * i / ivc_capacitor.length)

    def test_error_message(self):
        ivc_resistor_1 = IvCurve()
        for i in range(MAX_NUM_POINTS):
            ivc_resistor_1.length = MAX_NUM_POINTS
            ivc_resistor_1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            ivc_resistor_1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)

        # An error will be printed to stdout
        # and it is not easy to get it here
        print("VVV  The following error messages are a part of test. Don’t care.  VVV")
        SetMinVarVC(0, 0)
        with self.assertRaises(RuntimeError):
            CompareIvc(ivc_resistor_1, ivc_resistor_1)
        print("^^^ Error testing finished. In case there are any error messages below, it’s a problem. ^^^")

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
