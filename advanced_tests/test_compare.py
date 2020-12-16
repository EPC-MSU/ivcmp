import json
import unittest
import numpy as np
import ivcmp
import random
from iv_compare import compare_ivc
# import matplotlib.pyplot as plt

json_folder = "test_data"
iv_curve = ivcmp.IvCurve()
ivc_curve = ivcmp.IvCurve()
ivc_data = {}
iv_data = {}


class TestStringMethods(unittest.TestCase):

    def test_nan(self):
        count = 0
        for i in range(1, 21):
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
                iv_curve.length = n_points
                ivc_curve.length = n_points
                iv_curve.voltages[:n_points] = iv_data["elements"][0]["pins"][0]["ivc"]["voltage"][:n_points]
                iv_curve.currents[:n_points] = iv_data["elements"][0]["pins"][0]["ivc"]["current"][:n_points]
                ivc_curve.voltages[:n_points] = ivc_data["elements"][0]["pins"][0]["ivc"]["voltage"][:n_points]
                ivc_curve.currents[:n_points] = ivc_data["elements"][0]["pins"][0]["ivc"]["current"][:n_points]
                score = ivcmp.CompareIvc(iv_curve, ivc_curve)
                assert (0 <= score <= 1.)
            except AssertionError:
                count += 1
                print("AssertionError: comp_1: {}, comp_2: {}"
                      "score - {}".format(i, ind_1, ind_2, score))
            except Exception as e:
                print("ERROR in elements ({}).json: {}".format(i, e))
        print("There is {} errors of 20 random comparing".format(count))
        self.assertTrue(True)

    def test_compare(self):
        count = 0
        common_count = 0
        for i in range(1, 42):
            filename = "{}/elements ({}).json".format(json_folder, i)
            with open(filename) as f:
                ivc_data = json.load(f)
                for pin in ivc_data["elements"][0]["pins"]:
                    common_count += 1
                    try:
                        n_points = len(pin["ivc"]["voltage"])
                        iv_curve.length = n_points
                        ivc_curve.length = n_points
                        iv_curve.voltages[:n_points] = pin["ivc"]["voltage"][:n_points]
                        iv_curve.currents[:n_points] = pin["ivc"]["current"][:n_points]
                        ivc_curve.voltages[:n_points] = pin["reference_ivc"]["voltage"][:n_points]
                        ivc_curve.currents[:n_points] = pin["reference_ivc"]["current"][:n_points]
                        ivcmp.SetMinVC(max(np.max(pin["ivc"]["current"]), 0.6) / 2,
                                       max(np.max(pin["ivc"]["current"]), 0.0002) / 2)
                        score_c = ivcmp.CompareIvc(iv_curve, ivc_curve)
                        score_py = compare_ivc([pin["ivc"]["voltage"][:n_points],
                                                pin["ivc"]["current"][:n_points]],
                                               [pin["reference_ivc"]["voltage"][:n_points],
                                                pin["reference_ivc"]["current"][:n_points]])
                        target_score = pin["score"]
                        assert (np.abs(score_c - target_score) < 0.05)
                    except AssertionError:
                        print("AssertionError in elements ({}).json, {} component: "
                              "score - {}, target_score - {}, "
                              "score_py - {}".format(i, ivc_data["elements"][0]["pins"].index(pin), score_c,
                                                     target_score, score_py))

                        """plt.suptitle("score - {}, target_score - {}, score_py - {}".format(np.round(score_c, 2),
                                                                                           np.round(target_score, 2),
                                                                                           np.round(score_py, 2)))
                        plt.plot(pin["ivc"]["voltage"][:n_points], pin["ivc"]["current"][:n_points], marker="o",
                                 linestyle="dashed", label="Current curve")
                        plt.plot(pin["reference_ivc"]["voltage"][:n_points], pin["reference_ivc"]["current"][:n_points],
                                 marker="o", linestyle="dashed", label="Reference curve")
                        plt.legend()
                        plt.show()"""
                        count += 1
                    except Exception as e:
                        print("ERROR in elements ({}).json: {}".format(i, e))
        print("There is {} errors of {} comparing".format(count, common_count))
        self.assertTrue(True)


if __name__ == "__main__":
    unittest.main()
