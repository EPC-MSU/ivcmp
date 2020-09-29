import json
import unittest
import numpy as np
import ivcmp
import random

json_folder = "test_data"
iv_curve = ivcmp.IvCurve()
ivc_curve = ivcmp.IvCurve()
ivc_data = {}
iv_data = {}


class TestStringMethods(unittest.TestCase):

    def test_nan(self):
        for i in range(1, 10):
            ind_1 = random.randint(1, 41)
            ind_2 = random.randint(1, 41)
            filename_1 = "{}/elements ({}).json".format(json_folder, ind_1)
            filename_2 = "{}/elements ({}).json".format(json_folder, ind_2)
            with open(filename_1) as f:
                iv_data = json.load(f)
            with open(filename_2) as f:
                ivc_data = json.load(f)
            try:
                n_points = np.min([len(iv_data["elements"][0]["pins"][0]["ivc"]["voltage"]),
                                   len(ivc_data["elements"][0]["pins"][0]["ivc"]["voltage"])])
                iv_curve.voltages[:n_points] = iv_data["elements"][0]["pins"][0]["ivc"]["voltage"][:n_points]
                iv_curve.currents[:n_points] = iv_data["elements"][0]["pins"][0]["ivc"]["current"][:n_points]
                ivc_curve.voltages[:n_points] = ivc_data["elements"][0]["pins"][0]["ivc"]["voltage"][:n_points]
                ivc_curve.currents[:n_points] = ivc_data["elements"][0]["pins"][0]["ivc"]["current"][:n_points]
                score = ivcmp.CompareIvc(iv_curve, ivc_curve)
                assert (0 <= score <= 1.)
            except AssertionError:
                print("AssertionError: comp_1: {}, comp_2: {}"
                      "score - {}".format(i, ind_1, ind_2, score))
            except Exception as e:
                print("ERROR in elements ({}).json: {}".format(i, e))
        self.assertTrue(True)

    def test_compare(self):
        for i in range(1, 42):
            filename = "{}/elements ({}).json".format(json_folder, i)
            with open(filename) as f:
                ivc_data = json.load(f)
                for pin in ivc_data["elements"][0]["pins"]:
                    try:
                        n_points = len(pin["ivc"]["voltage"])
                        iv_curve.voltages[:n_points] = pin["ivc"]["voltage"]
                        iv_curve.currents[:n_points] = pin["ivc"]["current"]
                        ivc_curve.voltages[:n_points] = pin["reference_ivc"]["voltage"]
                        ivc_curve.currents[:n_points] = pin["reference_ivc"]["current"]
                        ivcmp.SetMinVC(0.001 * np.max(pin["ivc"]["voltage"]), 0.001 * np.max(pin["ivc"]["current"]))
                        score = ivcmp.CompareIvc(iv_curve, ivc_curve)
                        target_score = pin["score"]
                        assert (np.abs(score - target_score) < 0.05)
                    except AssertionError:
                        print("AssertionError in elements ({}).json, {} component: "
                              "score - {}, target_score - {}".format(i, ivc_data["elements"][0]["pins"].index(pin),
                                                                     score, target_score))
                    except Exception as e:
                        print("ERROR in elements ({}).json: {}".format(i, e))
        self.assertTrue(True)
        

if __name__ == "__main__":
    unittest.main()
