

Запуск ivcmp.py на Windows:
    Сгенерировать динамическую библиотеку (см. ivcmp-src/readme.md)
    Поместить рядом с ivcmp.py ivcmp.dll
    Запустить:
    python ivcmp.py


Запуск ivcmp.py на Linux:
    Сгенерировать динамическую библиотеку (см. ivcmp-src/readme.md)
    В ivcmp.py прописать путь к библиотеке: CDLL("path/to/libivcmp.so")
    export LD_LIBRARY_PATH=path/to/libivcmp.so
    Запустить:
    python libivcmp.py

    Аналогично для test_libivcmp.py
