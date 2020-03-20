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
 * Функция для установки параметров сравнения сигнатур. 
 * Параметры, устанавливаемые в данной функции, не влияют на результаты измерений 
 * и будут применяться только для учёта шумов при сравнении сигнатур.
 * @param[in] NewMinV Характерный уровень шума по напряжению. Единицы измерения: Вольты.
 * @param[in] NewMinC Характерный уровень шума по току. Единицы измерения: мА.
 */
EXPORT void CCONV SetMinVC(double NewMinV, double NewMinC);

/**
 * Функция для сравнения двух кривых (ВАХ). 
 * Возвращает степень различия (0 - кривые совпадают, 1  - кривые совсем разные). 
 * Токи и напряжения должны быть заданы в одних и тех же единицах измерения.
 * Массивы токов и напряжений должны двух кривых
 * должны иметь одинаковую дину и содержать по одному периоду пробного сигнала
 * (один цикл замкнутой кривой). 
 * @param[in] VoltagesA Массив напряжений первой кривой для сравнения
 * @param[in] CurrentsA Массив токов первой кривой для сравнения
 * @param[in] VoltagesB Массив напряжений второй кривой для сравнения
 * @param[in] CurrentsB Массив токов второй кривой для сравнения
 * @param[in] CurveLength Kоличество точек кривой
 * @param[out] Score Степень различия
 */
EXPORT double CCONV CompareIVC(double *VoltagesA, double *CurrentsA,
                               double *VoltagesB, double *CurrentsB,
                               uint32_t CurveLength);
#ifdef __cplusplus
}
#endif

#endif // IVCMP_H