import numpy as np
from scipy import interpolate
import matplotlib.pyplot as plt
import os
import sys
import json
from PyQt5.QtWidgets import QFileDialog
from PyQt5.QtWidgets import QWidget, QDesktopWidget, QApplication, QPushButton, QLabel
from PyQt5 import QtCore
import pylab

eq_k = 1


def dist2_pt_seg(p, a, b):
    v1 = np.subtract(b, a)
    v2 = np.subtract(p, a)
    seg_len2 = np.dot(v1, v1)
    proj = np.dot(v1, v2) / seg_len2
    if proj < 0:
        return np.dot(v2, v2)
    if proj > 1:
        return np.dot(np.subtract(p, b), np.subtract(p, b))

    return np.cross(v1, v2) ** 2 / seg_len2


v_dist2_pt_seg = np.vectorize(dist2_pt_seg)


def rescale_score(x):
    return 1 - np.exp(-8 * x)


def dist_curve_pts(curve, pts):
    res = 0.0
    for pt in pts.T:
        v = (curve[0] - pt[0]) * (curve[0] - pt[0]) + (curve[1] - pt[1]) * (curve[1] - pt[1])
        min_i = v.argmin()
        res += min(
            dist2_pt_seg(pt, curve[:, min_i - 1], curve[:, min_i]) if min_i > 0 else np.inf,
            dist2_pt_seg(pt, curve[:, min_i], curve[:, min_i + 1]) if min_i < len(
                curve[0]) - 1 else np.inf)
    res /= len(pts.T)
    return res


def remove_repeats_ivc(a, eps=1e-6):
    msk0 = np.append(np.abs(a[0][1:] - a[0][:-1]) > eps, True)
    msk1 = np.append(np.abs(a[1][1:] - a[1][:-1]) > eps, True)
    return a[0][msk0 | msk1], a[1][msk0 | msk1]


def compare_ivc(a, b=None, min_var_v=None, min_var_c=None):
    # a, b:  tuple(oscilloscope instance, curve)
    # curve: tuple(voltage_points, current_points)

    if a is None:
        return 0
    min_v = max(np.max(a[0]), 0.6)
    min_c = max(np.max(a[1]), 0.0002)
    # Now a and b - curves - tuple(voltage_points, current_points)

    if min_var_v is None:
        min_var_v = min_v / 2

    if min_var_c is None:
        min_var_c = min_c / 2

    # The variance is the average of the squared deviations from the mean, i.e., var = mean(abs(x - x.mean())**2).
    var_v = max(np.var(a[0]) ** 0.5, np.var(b[0]) ** 0.5 if b is not None else 0, min_var_v)
    var_c = max(np.var(a[1]) ** 0.5, np.var(b[1]) ** 0.5 if b is not None else 0, min_var_c)
    _eq_k = 1
    an = np.subtract(a[0], np.mean(a[0])) / var_v, np.subtract(a[1], np.mean(a[1])) / var_c
    an = remove_repeats_ivc(an)
    tck, u = interpolate.splprep(an, s=0.00)
    eq1 = np.array(interpolate.splev(np.arange(0, 1, 1.0 / len(a[0]) / _eq_k), tck))

    if b is not None:
        bn = np.subtract(b[0], np.mean(b[0])) / var_v, np.subtract(b[1], np.mean(b[1])) / var_c
        bn = remove_repeats_ivc(bn)
        tck, u = interpolate.splprep(bn, s=0.00)
        eq2 = np.array(interpolate.splev(np.arange(0, 1, 1.0 / len(b[0]) / _eq_k), tck))
    if b is None:
        return rescale_score(np.mean(eq1[1, :] ** 2))
    else:
        return rescale_score((dist_curve_pts(eq1, eq2) + dist_curve_pts(eq2, eq1)) / 2.)


class MainWindow(QWidget):

    def __init__(self):
        super().__init__()

        self.initUI()

    def initUI(self):

        self.resize(300, 150)
        self.center()
        self.qbtn = QPushButton("Compare curves", self)
        self.lbl = QLabel(self)
        self.lbl.move(75, 100)
        self.qbtn.clicked.connect(self.main)
        self.qbtn.resize(self.qbtn.sizeHint())
        self.qbtn.move(100, 50)
        self.setWindowTitle("Comparing")
        self.show()

    def center(self):

        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    @QtCore.pyqtSlot(bool)
    def main(self):
        real_path = QFileDialog.getOpenFileName(
            caption="Выберите json-файл",
            directory=os.path.join("./", "ivc_1"),
            filter="JSON file (*.json)"
        )[0]
        virt_path = QFileDialog.getOpenFileName(
            caption="Выберите json-файл",
            directory=os.path.join("./", "ivc_2"),
            filter="JSON file (*.json)"
        )[0]
        if real_path != "" and virt_path != "":
            with open(real_path, "r") as dump_file:
                ivc_real = json.load(dump_file)
            with open(virt_path, "r") as dump_file:
                ivc_virt = json.load(dump_file)
            score = calc_score(ivc_real, ivc_virt)
            self.lbl.setText("Score: " + str(score))
            self.lbl.adjustSize()
            plt.figure(1, (10, 5))
            plt.plot(ivc_real["ivc"]["voltages"], ivc_real["ivc"]["currents"], color="g")
            plt.plot(ivc_virt["ivc"]["voltages"], ivc_virt["ivc"]["currents"], color="r")
            plt.xlabel("Напряжение [В]")
            plt.ylabel("Сила тока [А]")
            max_v = max(np.max(ivc_virt["ivc"]["voltages"]), np.max(ivc_real["ivc"]["voltages"]))
            max_c = max(np.max(ivc_virt["ivc"]["currents"]), np.max(ivc_real["ivc"]["currents"]))
            min_v = min(np.min(ivc_virt["ivc"]["voltages"]), np.min(ivc_real["ivc"]["voltages"]))
            min_c = min(np.min(ivc_virt["ivc"]["currents"]), np.min(ivc_real["ivc"]["currents"]))
            pylab.xlim(1.4 * min_v, 1.4 * max_v)
            pylab.ylim(1.4 * min_c, 1.4 * max_c)
            plt.show()
        else:
            self.lbl.setText("No curves selected!")
            self.lbl.adjustSize()


def calc_score(ivc_real, ivc_virt):
    r_ivc = [ivc_real["ivc"]["voltages"], ivc_real["ivc"]["currents"]]
    v_ivc = [ivc_virt["ivc"]["voltages"], ivc_virt["ivc"]["currents"]]
    score = compare_ivc(r_ivc, v_ivc)
    return score


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ex = MainWindow()
    sys.exit(app.exec_())
