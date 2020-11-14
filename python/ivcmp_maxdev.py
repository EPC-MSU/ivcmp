from ctypes import CDLL, Structure, Array, c_ubyte, \
    c_double, c_size_t, POINTER, byref
from platform import system
import numpy as np
VOLTAGE_AMPL = 12.
R_CS = 475.
CURRENT_AMPL = (VOLTAGE_AMPL / R_CS * 1000)


def _get_dll():
    if system() == "Linux":
        return CDLL("libivcmp_maxdev.so")
    elif system() == "Windows":
        return CDLL("ivcmp_maxdev.dll")
    else:
        raise NotImplementedError("Unsupported platform {0}".format(system()))


lib = _get_dll()
MAX_NUM_POINTS = 1000


class _IterableStructure(Structure):
    def __iter__(self):
        return (getattr(self, n) for n, t in self._fields_)


def _normalize_arg(value, desired_ctype):
    from collections import Sequence

    if isinstance(value, desired_ctype):
        return value
    elif issubclass(desired_ctype, Array) and isinstance(value, Sequence):
        member_type = desired_ctype._type_

        if desired_ctype._length_ < len(value):
            raise ValueError()

        if issubclass(member_type, c_ubyte) and isinstance(value, bytes):
            return desired_ctype.from_buffer_copy(value)
        elif issubclass(member_type, c_ubyte) and isinstance(value, bytearray):
            return value
        else:
            return desired_ctype(*value)
    else:
        return value


class IvCurve(_IterableStructure):
    _fields_ = (
        ("voltages", c_double*MAX_NUM_POINTS),
        ("currents", c_double*MAX_NUM_POINTS),
        ("length", c_size_t)
    )

    def __init__(self):
        self.length = MAX_NUM_POINTS


def ComputeMaxDeviations(first_iv_curve, second_iv_curve):
    dev_v, dev_c = c_double(0.0), c_double(0.0)
    lib_func = lib.ComputeMaxDeviations
    lib_func.argtype = POINTER(c_double), POINTER(c_double), c_size_t, POINTER(c_double),\
        POINTER(c_double), c_size_t, POINTER(c_double), POINTER(c_double)
    lib_func.restype = None
    lib_func(first_iv_curve.voltages, first_iv_curve.currents, first_iv_curve.length,
                   second_iv_curve.voltages, second_iv_curve.currents, second_iv_curve.length,
                   byref(dev_v), byref(dev_c))
    dev_v = _normalize_arg(dev_v, c_double)
    dev_c = _normalize_arg(dev_c, c_double)
    return dev_v.value, dev_c.value


if __name__ == "__main__":
    iv_curve = IvCurve()
    ivc_curve = IvCurve()
    for i in range(MAX_NUM_POINTS):
        iv_curve.voltages[i] = 0.47 * VOLTAGE_AMPL * np.sin(2 * 3.14 * i / MAX_NUM_POINTS)
        iv_curve.currents[i] = 0.63 * CURRENT_AMPL * np.sin(2 * 3.14 * i / MAX_NUM_POINTS)
        ivc_curve.voltages[i] = VOLTAGE_AMPL * np.sin(2 * 3.14 * i / MAX_NUM_POINTS)
        ivc_curve.currents[i] = CURRENT_AMPL * np.cos(2 * 3.14 * i / MAX_NUM_POINTS)
    dev_v, dev_c = ComputeMaxDeviations(iv_curve, ivc_curve)
    print(dev_v, dev_c)
    # for i in range(MAX_NUM_POINTS):
    #     print(iv_curve.currents[i], iv_curve.voltages[i]).
