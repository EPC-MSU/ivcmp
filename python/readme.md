

Запуск libivcmp.py на Windows:
    Сгенерировать динамическую библиотеку с помощью ivcmp.sln (см. ivcmp-src/readme.md)
    Поместить рядом с libivcmp.py ivcmp.dll
    Запустить:
    python libivcmp.py


Запуск libivcmp.py на Linux:
    Сгенерировать динамическую библиотеку с помощью Makefile (см. ivcmp-src/readme.md)
    В libivcmp.py прописать путь к библиотеке: CDLL("../build/libivcmp.so")
    export LD_LIBRARY_PATH=../build/libivcmp.so
    Запустить:
    python3 libivcmp.py

    Аналогично для test_libivcmp.py
