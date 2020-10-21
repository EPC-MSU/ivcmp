/** \file ivcmp_maxdev.h
 * Библиотека для вычисления максимальных отклонений ВАХ.
 */

#ifndef IVCMP_MAXDEV_H
#define IVCMP_MAXDEV_H

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
 * Функция для определения максимальных отклонений двух кривых (ВАХ).
 * Рассчитывает максимальные расстояния между пробной кривой и тестовой,
 * затем нормализует по максимуму пробной кривой.
 * @param[in] VoltagesA Массив напряжений пробной кривой для сравнения [Вольты]
 * @param[in] CurrentsA Массив токов пробной кривой для сравнения [мА]
 * @param[in] VoltagesB Массив напряжений тестовой кривой для сравнения [Вольты]
 * @param[in] CurrentsB Массив токов тестовой кривой для сравнения [мА]
 * @param[in] CurveLength Kоличество точек кривой
 * @param ScoreV Максимальное отклонение по напряжениям
 * @param ScoreC Максимальное отнлонение по токам
 */
EXPORT void CCONV ComputeMaxDeviations(double *VoltagesA, double *CurrentsA, uint32_t CurveLengthA,
                                         double *VoltagesB, double *CurrentsB, uint32_t CurveLengthB,
                                         double ScoreV, double ScoreC);
#ifdef __cplusplus
}
#endif

#endif /* IVCMP_MAXDEV_H */