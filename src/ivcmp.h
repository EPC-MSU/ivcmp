/** \file ivcmp.h
 * Библиотека для сравнения ВАХ.
 */

#ifndef IVCMP_H
#define IVCMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#if defined(_WIN32) || defined (_WIN64)
#define EXPORT __declspec(dllexport)
#define CCONV __cdecl
#else
#define EXPORT
#define CCONV
#endif

/** 
 * Обязательная функция для установки параметров сравнения сигнатур. 
 * Параметры, устанавливаемые в данной функции, не влияют на результаты измерений 
 * и будут применяться только для учёта шумов при сравнении сигнатур.
 * По сути это задаёт минимальный масштаб, на котором рассматривается похожесть
 * форм двух сигнатур.
 * Если не задать эти параметры, то 2 горизонтальные или 2 вертикальные сигнатуры
 * будут всегда различны, так как они шумят и при масштабировании шумящая линия
 * уширяется до границ изображения. 2 разных шума, 2 разных сигнатуры.
 * @param[in] NewMinV Характерный уровень шума по напряжению. Единицы измерения: Вольты.
 * @param[in] NewMinC Характерный уровень шума по току. Единицы измерения: мА.
 */
EXPORT void CCONV SetMinVC(double NewMinV, double NewMinC);

/**
 * Функция для сравнения двух кривых (ВАХ).
 * Возвращает степень различия в диапазоне [0, 1]
 * (0 - кривые совпадают, 1  - кривые совсем разные).
 * Степень различия соответствует визуальному различию кривых, нанесённых 
 * на одни оси напряжение-ток. Различными кривыми будут круг и круг с 
 * другим радиусом (чем больше различие радиусов, тем больше отличие), 
 * отрезок прямой с одним наклоном и с отрезок с другим наклоном или с 
 * другой длиной, ну и тем более отрезок и эллипс (хотя если эллипс 
 * ложится на отрезок и вытянут вдоль него, то степень различия будет небольшой).
 * Массивы токов и напряжений должны двух кривых
 * должны иметь одинаковую длину и содержать по одному периоду пробного сигнала
 * (один цикл замкнутой кривой). 
 * @param[in] VoltagesA Массив напряжений первой кривой для сравнения [Вольты]
 * @param[in] CurrentsA Массив токов первой кривой для сравнения [мА]
 * @param[in] VoltagesB Массив напряжений второй кривой для сравнения [Вольты]
 * @param[in] CurrentsB Массив токов второй кривой для сравнения [мА]
 * @param[in] CurveLength Kоличество точек кривой
 * @param[out] Score Степень различия
 */
EXPORT double CCONV CompareIVC(double *VoltagesA, double *CurrentsA, uint32_t CurveLengthA,
                               double *VoltagesB, double *CurrentsB, uint32_t CurveLengthB);

/**
* Функция для определения максимальных отклонений двух кривых (ВАХ).
* Рассчитывает максимальные расстояния между пробной кривой и тестовой,
* затем нормализует по максимуму пробной кривой.
* Массивы токов и напряжений должны иметь одинаковые длины и содержать по
* одному периоду пробного сигнала.
* @param[in] VoltagesA Массив напряжений пробной кривой для сравнения [Вольты]
* @param[in] CurrentsA Массив токов пробной кривой для сравнения [мА]
* @param[in] VoltagesB Массив напряжений тестовой кривой для сравнения [Вольты]
* @param[in] CurrentsB Массив токов тестовой кривой для сравнения [мА]
* @param[in] CurveLength Kоличество точек кривой
* @param ScoreV Максимальное отклонение по напряжениям
* @param ScoreC Максимальное отнлонение по токам
*/
EXPORT void CCONV ComputeMaxDeviations(double *VoltagesRef, double *CurrentsRef, uint32_t CurveLengthRef,
	double *VoltagesTest, double *CurrentsTest, uint32_t CurveLengthTest,
	double *ScoreV, double *ScoreC);

#ifdef __cplusplus
}
#endif

#endif /* IVCMP_H */
