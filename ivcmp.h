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
 * Функция установки характерного масштаба измерений
 * Для корректной работы библиотеки обязательно требуется установить
 * характерные масштабы измерений. Во всех реальных измерениях присутствуют шумы.
 * Шумы влияют на результат сравнения кривых.
 * Если шумы не учитывать, то при сравнении визуально совпадающих кривых,
 * библиотека может выдать ненулевую степень различия из-за неправильной нормировки.
 * Чтобы этого не произошло, установите характерные масштабы измерений с помощью данной функции.
 * Сохранённые значения будут использоваться для всех последующих сравнений кривых
 * до тех пор пока масштабы не будут обновлены путём вызова данной функции 
 * или функции SetMinVCFromCurves().
 * Если Вы не знаете, как определить характерные масштабы измерений, 
 * но у Вас есть сигнатуры разрыва и короткого замыкания, Вы можете воспользоваться 
 * функцией SetMinVCFromCurves(), которая автоматически определяет и устанавливает 
 * характерные масштабы на основе измерений.
 * Другим простым способом установки масштаба измерений будет деление диапазона измерений на 30.
 * Например, если диапазон измерений по напряжению 12В, то NewMinV должен быть установлен в 0.4В.
 * То же самое с токами. Если максимальный ток на этом режиме неизвестен, то можно его измерить с
 * помощью короткого замыкания щупов.
 * @param[in] NewMinV Характерный масштаб по напряжению. Единицы измерения: Вольты.
 * @param[in] NewMinC Характерный масштаб по току. Единицы измерения: мА.
 */
EXPORT void CCONV SetMinVC(double NewMinV, double NewMinC);

/**
 * Функция определения и установки характерного масштаба измерений 
 * на основе калибровочных сигнатур (ВАХ). Функция принимает две сигнатуры: 
 * первая должна быть измеренна при разомкнутрых щупах,
 * вторая должна быть измеренна в случае, когда щупы замкнуты накоротко.
 * На основе полученных сигнатур функция определяет характерные масштабы измерений
 * и сохраняет их для последующего использования.
 * Сохранённые значения будут использоваться для всех последующих сравнений кривых
 * до тех пор пока масштабы не будут обновлены путём вызова данной функции 
 * или функции SetMinVC().
 * Данная функция является альтернативой для функции SetMinVC().
 * Если Вы не знаете уровень шумов, но у Вас есть сигнатуры короткого замыкания
 * и разрыва, используйте эту функцию. Если Вы знаете уровни шумов по току
 * и напряжению, используйте функцию SetMinVC().
 * Использовать обе функции одновременно не нужно.
 * @param[in] VoltagesOpenC Массив напряжений сигнатуры, снятой при разомкнутых щупах [Вольты]
 * @param[in] CurrentsOpenC Массив токов сигнатуры, снятой при разомкнутых щупах [мА]
 * @param[in] CurveLengthOpenC Kоличество элементов в массивах VoltagesOpenC и CurrentsOpenC (должно быть одинаковым).
 * @param[in] VoltagesShortC Массив напряжений сигнатуры, снятой при коротко замкнутых щупах [Вольты]
 * @param[in] CurrentsShortC Массив токов сигнатуры, снятой при коротко замкнутых щупах [Вольты]
 * @param[in] CurveLengthShotC Kоличество элементов в массивах VoltagesShortC и CurrentsShortC (должно быть одинаковым).
 */
EXPORT void CCONV SetMinVCFromCurves(double *VoltagesOpenC, double *CurrentsOpenC, uint32_t CurveLengthOpenC,
                                     double *VoltagesShortC, double *CurrentsShortC, uint32_t CurveLengthShotC);

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
 * @param[in] CurveLengthA Kоличество элементов в массивах VoltagesA и CurrentsA (должно быть одинаковым).
 * @param[in] VoltagesB Массив напряжений второй кривой для сравнения [Вольты]
 * @param[in] CurrentsB Массив токов второй кривой для сравнения [мА]
 * @param[in] CurveLengthB Kоличество элементов в массивах VoltagesA и CurrentsA (должно быть одинаковым).
 * @param[out] Score Степень различия
 */
EXPORT double CCONV CompareIVC(double *VoltagesA, double *CurrentsA, uint32_t CurveLengthA,
                               double *VoltagesB, double *CurrentsB, uint32_t CurveLengthB);
#ifdef __cplusplus
}
#endif

#endif /* IVCMP_H */
