import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np

root_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(root_dir))
from pyivcmp.ivcmp import CompareIvc, IvCurve, MAX_NUM_POINTS, SetMinVarVC, SetRangesVC


def create_curve(voltage: float, current: float, points_number: int) -> IvCurve:
    iv_curve = IvCurve()
    iv_curve.length = points_number
    for i in range(points_number):
        iv_curve.voltages[i] = voltage * np.sin(2 * 3.14 * i / points_number)
        iv_curve.currents[i] = current * np.sin(2 * 3.14 * i / points_number)

    return iv_curve


def main():
    voltage_range = 12
    current_range = 5
    curve_a = create_curve(voltage_range, current_range, MAX_NUM_POINTS)
    curve_b = create_curve(0.9 * voltage_range, 0.5 * current_range, MAX_NUM_POINTS)

    # Set curves scale and range
    SetMinVarVC(0.03 * voltage_range, 0.03 * current_range)
    SetRangesVC(voltage_range, current_range)

    score = CompareIvc(curve_a, curve_b)
    plot_curves(curve_a, curve_b)
    print(score)


def plot_curves(curve_a: IvCurve, curve_b: IvCurve) -> None:
    """
    Строит две кривые класса IvCurve на одном графике.

    :param curve_a: Объект класса IvCurve (первая кривая).
    :param curve_b: Объект класса IvCurve (вторая кривая).
    """


    voltages_a = list(curve_a.voltages[:curve_a.length])
    currents_a = list(curve_a.currents[:curve_a.length])
    voltages_b = list(curve_b.voltages[:curve_b.length])
    currents_b = list(curve_b.currents[:curve_b.length])

    plt.figure(figsize=(10, 6))

    plt.plot(voltages_a, currents_a, label="Кривая А", color="blue", marker="o", linestyle="-", markersize=4)
    plt.plot(voltages_b, currents_b, label="Кривая Б", color="red", marker="x", linestyle="--", markersize=5)

    plt.xlabel("Напряжение, В", fontsize=12)
    plt.ylabel("Ток, мА", fontsize=12)
    plt.grid(True, linestyle=":", alpha=0.6)
    plt.legend(fontsize=11, loc="best")
    plt.show()


if __name__ == "__main__":
    main()
