"""
This script make debug data plots and save them as .png images.

To obtain plots:
1. Uncomment DEBUG_FILE_OUTPUT macro in library source file.
2. Rebuild the library.
3. Compare two curves with the built library. The library will produce a set of text files with debug data.
4. Place this script into folder with files with plot data.
5. Run this script with python.
"""

import matplotlib.pyplot as plt
import numpy as np

def read_curve_from_file(file_name):
    voltages = []
    currents = []
    with open(file_name, "r") as f:
        lines = f.readlines()

    for line in lines:
        v_str, i_str = line.split()
        voltages.append(float(v_str))
        currents.append(float(i_str))

    return voltages, currents

def plot_two_curves(plot_name, plot_title, plotfile_prefix=""):
    a_v, a_i = read_curve_from_file(plot_name + "_a.txt")
    b_v, b_i = read_curve_from_file(plot_name + "_b.txt")

    plt.figure(figsize=(12, 5))
    plt.suptitle(plot_title)

    plt.subplot(1, 2, 1)
    plt.title("Signals")
    plt.plot(a_v, marker="o", label="Voltages A")
    plt.plot(b_v, marker="o", label="Voltages B")
    plt.plot(a_i, marker="o", label="Currents A")
    plt.plot(b_i, marker="o", label="Currents B")
    plt.legend()
    plt.xlabel("Point number")
    plt.ylabel("Signal")

    plt.subplot(1, 2, 2)
    plt.title("IV Curve")
    plt.plot(a_v, a_i, marker="o", label="A")
    plt.plot(b_v, b_i, marker="o", label="B")
    plt.legend()
    plt.xlabel("Voltage")
    plt.ylabel("Current")
    
    plotfile_name = plot_name + ".png"
    if plotfile_prefix != "":
        plotfile_name = plotfile_prefix + "_" + plotfile_name

    plt.savefig(plotfile_name)

    #plt.show()

plot_two_curves("input_curve", "Входные сигналы", "1")
plot_two_curves("copied_curve", "Скопированные кривые", "2")
plot_two_curves("scaled", "Кривые после масштабирования", "3")
plot_two_curves("repeats_removed", "Кривые после устранения повторов", "4")
plot_two_curves("splined", "После равномерного распределения точек", "5")

