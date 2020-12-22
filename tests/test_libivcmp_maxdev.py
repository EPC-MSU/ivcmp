# This module tests libicvmp_maxdev and checks if binding works correctly
from __future__ import print_function
import unittest
from ivcmp import IvCurve, ComputeMaxDeviations, MAX_NUM_POINTS, VOLTAGE_AMPL, CURRENT_AMPL
from ctypes import c_double
import numpy as np


class TestStringMethods(unittest.TestCase):

    def test_number_one(self):
        self.IVCResistor1 = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
        resV, resC = ComputeMaxDeviations(self.IVCResistor1, self.IVCResistor1)
        self.assertTrue((resV < 0.01) and (resC < 0.01))

    def test_number_two(self):
        self.IVCOpenCircuit = IvCurve()
        self.IVCShortCircuit = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCOpenCircuit.voltages[i] = c_double(VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCOpenCircuit.currents[i] = c_double(0)
            self.IVCShortCircuit.voltages[i] = c_double(0)
            self.IVCShortCircuit.currents[i] = c_double(CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
        resV, resC = ComputeMaxDeviations(self.IVCOpenCircuit, self.IVCShortCircuit)
        self.assertTrue(((resV - 0.99) < 0.02) and (resC < 0.01))

    def test_number_three(self):
        self.IVCOpenCircuit = IvCurve()
        self.IVCShortCircuit = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCOpenCircuit.voltages[i] = c_double(VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCOpenCircuit.currents[i] = c_double(0)
            self.IVCShortCircuit.voltages[i] = c_double(0)
            self.IVCShortCircuit.currents[i] = c_double(CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
        resV, resC = ComputeMaxDeviations(self.IVCShortCircuit, self.IVCOpenCircuit)
        self.assertTrue((resV < 0.01) and ((resC - 0.99) < 0.02))

    def test_number_four(self):
        self.IVCResistor1 = IvCurve()
        self.IVCResistor2 = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = c_double(0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor1.currents[i] = c_double(0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor2.voltages[i] = c_double(0.47 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
            self.IVCResistor2.currents[i] = c_double(0.63 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS))
        resV, resC = ComputeMaxDeviations(self.IVCResistor1, self.IVCResistor2)
        self.assertTrue(((resV - 0.14) < 0.03) and ((resC - 0.16) < 0.03))

    def test_number_five(self):
        self.IVCResistor1 = IvCurve()
        self.IVCCapacitor = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCResistor1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCCapacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCCapacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / MAX_NUM_POINTS)
        resV, resC = ComputeMaxDeviations(self.IVCResistor1, self.IVCCapacitor)
        self.assertTrue(((resV - 1.98) < 0.05) and ((resC - 1.98) < 0.05))

    def test_number_six(self):
        self.IVCResistor1 = IvCurve()
        self.IVCCapacitor = IvCurve()
        for i in range(MAX_NUM_POINTS):
            self.IVCResistor1.voltages[i] = 0.5 * VOLTAGE_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
            self.IVCResistor1.currents[i] = 0.5 * CURRENT_AMPL * np.sin(2 * np.pi * i / MAX_NUM_POINTS)
        self.IVCCapacitor.length = 20
        for i in range(self.IVCCapacitor.length):
            self.IVCCapacitor.voltages[i] = VOLTAGE_AMPL * np.sin(2 * np.pi * i / self.IVCCapacitor.length)
            self.IVCCapacitor.currents[i] = CURRENT_AMPL * np.cos(2 * np.pi * i / self.IVCCapacitor.length)
        resV, resC = ComputeMaxDeviations(self.IVCResistor1, self.IVCCapacitor)
        self.assertTrue(((resV - 1.74) < 0.05) and ((resC - 1.78) < 0.05))


if __name__ == "__main__":
    unittest.main()
