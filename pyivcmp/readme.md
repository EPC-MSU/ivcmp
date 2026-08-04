## Запуск ivcmp.py на Windows

* Сгенерировать динамическую библиотеку `ivcmp.dll` (см. `../readme.md`).
* Поместить рядом с `ivcmp.py` файл `ivcmp.dll`.
* Установить зависимости:

  ```commandline
  python -m venv venv
  venv\Scripts\python -m pip install --upgrade pip
  venv\Scripts\python -m pip install -r requirements.txt
  ```

* Запустить:

  ```commandline
  venv\Scripts\python ivcmp.py
  ```

## Запуск ivcmp.py на Linux

* Сгенерировать динамическую библиотеку `libivcmp.so` (см. `../readme.md`).
* Поместить рядом с `ivcmp.py` файл `libivcmp.so`.
* Установить зависимости:

  ```bash
  python3 -m venv venv
  venv/bin/python -m pip install --upgrade pip
  venv/bin/python -m pip install -r requirements.txt
  ```

* Выполнить в консоли команду:

  ```bash
  export LD_LIBRARY_PATH=path/to/libivcmp.so
  ```

* Запустить в этой же консоли:

  ```bash
  venv/bin/python ivcmp.py
  ```

## Запуск тестов

```
cd ..
python -m pip install pytest
python -m pytest pyivcmp/tests
```

### Важно!!!

При работе с библиотекой `ivcmp` не забывайте указывать реальную длину кривой в поле `length` объектов класса `IvCurve()`. Если это значение не задать явно, результаты сравнения могут оказаться не совсем корректные, поскольку для сравнения могут использоваться точки, лежащие за границами переданных массивов.
