

Запуск ivcmp.py на Windows:
    Сгенерировать динамическую библиотеку ivcmp.dll (см. ../readme.md)
    Поместить рядом с ivcmp.py ivcmp.dll
    Запустить:
    python ivcmp.py


Запуск ivcmp.py на Linux:
    Сгенерировать динамическую библиотеку livivcmp.so (см. ../readme.md)
    В ivcmp.py прописать абсолютный путь к библиотеке: CDLL("path/to/libivcmp.so")
    Выполнить в консоли команду `export LD_LIBRARY_PATH=path/to/libivcmp.so`
    Запустить в этой же консоли:
    python3 libivcmp.py

    Аналогично для test_libivcmp.py
