import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Circle

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
    curve_a = create_curve(voltage_range, current_range, 100)
    curve_b = create_curve(0.9 * voltage_range, 0.5 * current_range, 100)

    # Set curves scale and range
    SetMinVarVC(0.03 * voltage_range, 0.03 * current_range)
    SetRangesVC(voltage_range, current_range)

    score = CompareIvc(curve_a, curve_b)
    print(score)
    plot_curves(curve_a, curve_b)

    norm_curve_a = normalize_curve(curve_a, voltage_range, current_range)
    norm_curve_b = normalize_curve(curve_b, voltage_range, current_range)
    plot_curves_with_radius(norm_curve_a, norm_curve_b, score)


def normalize_curve(curve: IvCurve, voltage_range: float, current_range: float) -> IvCurve:
    normalized_curve = IvCurve()
    normalized_curve.length = curve.length
    for i in range(curve.length):
        normalized_curve.voltages[i] = curve.voltages[i] / voltage_range
        normalized_curve.currents[i] = curve.currents[i] / current_range

    return normalized_curve


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


def plot_curves_with_radius(curve_a, curve_b, radius, title="Сравнение ВАХ с окрестностью"):
    """
    Строит две кривые IvCurve и рисует окружности заданного радиуса вокруг точек кривой А.

    :param curve_a: Объект класса IvCurve (первая кривая)
    :param curve_b: Объект класса IvCurve (вторая кривая)
    :param radius: Радиус окружностей в единицах осей графика (например, 0.05)
    :param title: Заголовок графика
    """
    # Срезаем массивы ctypes по реальной длине
    voltages_a = list(curve_a.voltages[:curve_a.length])
    currents_a = list(curve_a.currents[:curve_a.length])

    voltages_b = list(curve_b.voltages[:curve_b.length])
    currents_b = list(curve_b.currents[:curve_b.length])

    # Создаем график и получаем объект осей (ax), необходимый для добавления патчей
    fig, ax = plt.subplots(figsize=(10, 6))

    # Рисуем линии и центры точек кривых
    ax.plot(voltages_a, currents_a, label="Кривая А (центры)", color="blue", marker="o", linestyle="-", markersize=4,
            zorder=3)
    ax.plot(voltages_b, currents_b, label="Кривая Б", color="red", marker="x", linestyle="--", markersize=5, zorder=4)

    # Добавляем геометрические окружности вокруг каждой точки кривой А
    for v, c in zip(voltages_a, currents_a):
        # facecolor='none' делает окружность прозрачной внутри
        # zorder=2 кладет окружности под линии маркеров, чтобы они не перекрывали центры точек
        circle = Circle((v, c), radius=radius, edgecolor="blue", facecolor="none", linestyle=":", alpha=0.5, zorder=2)
        ax.add_patch(circle)

    # Настраиваем равный масштаб осей, иначе окружности визуально сожмутся в эллипсы
    ax.set_aspect('equal', adjustable='datalim')

    # Оформление
    ax.set_title(title, fontsize=14, fontweight='bold')
    ax.set_xlabel("Напряжение, [В]", fontsize=12)
    ax.set_ylabel("Ток, [мА]", fontsize=12)
    ax.grid(True, linestyle=":", alpha=0.6)
    ax.legend(fontsize=11, loc="best")

    plt.show()


if __name__ == "__main__":
    main()
