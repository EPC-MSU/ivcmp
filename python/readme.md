

Запуск libivcmp.py на Windows:
    Сгенерировать динамическую библиотеку с помощью ivcmp.sln (см. ../readme.md)
    Поместить рядом с libivcmp.py ivcmp.dll
    Запустить:
    python libivcmp.py


Запуск libivcmp.py на Linux:
    Сгенерировать динамическую библиотеку с помощью Makefile (см. ../readme.md)
    В libivcmp.py прописать путь к библиотеке: CDLL("/home/user/.../libivcmp.so")
    export LD_LIBRARY_PATH=/path/to/libivcmp
    Запустить:
    python libivcmp.py

    Аналогично для test_libivcmp.py