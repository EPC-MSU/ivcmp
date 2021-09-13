# libivcmp

[![Coverage Badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/mihalin/b0ac32a32f4c12abc822ba5b31882f3f/raw/ivcmp__heads_main.json)]

Библиотека для сравнения кривых ВАХ. 

## Инструкция для Linux

###1. Установка cmake
```
sudo apt install cmake
```


### 2. Сборка и установка

Для сборки удобно создать отдельную папку в папке проекта:
```
mkdir build
cd build
```

Теперь сгенерируем make-файлы в эту папку.
```
cmake ..
```

Соберём проект:
```
make
```

Готово. В папке build появится библиотека libivcmp.so и исполняемый файл ivcmpexample;


Запуск:
```
./ivcmpexample
```


## Инструкция для Windows:

Для сборки под Windows понадобятся CMake, MinGW (или msvc) и Redistributable Packages 2013. 

Под Windows для генерации файлов проектом удобно использовать CMake Gui https://cmake.org/download/

* В CMake Gui в поле "Where is the source code" указать полный путь до папки проекта ivcmp

* В CMake Gui в поле  "Where to build binaries" указать полный путь до каталога, куда положить сборку. Например, это может быть папка build в каталоге проекта

* В CMake Gui нажать кнопку "Configure". В новом окне программа предложит выбрать генератор проекта. Можно выбрать либо MinGW, либо Visual Studio

Далее, в зависимости от желаемого типа проекта:

*Для Visual Studio 2013*
* Выбрать Visual Studio 2013, нажать Finish.
* Нажать Generate, затем Open project

* Появится окно Visual Studio с проектом. Дальше собрать кнопкой Build, как обычный MSVC проект

* Готово. Можно запускать собранный пример ivcmpexample.exe; библиотека ivcmp.dll, ivcmp.lib рядом

*Для MinGW*

* Выбрать MinGW, нажать Finish

* Нажать кнопку Generate

* Открыть консоль, встать в директорию со сгенерированными файлами

* Выполнить сборку mingw:
    ```
    mingw32-make
    ```
(путь до mingw должен быть в системных путях)

Готово. Можно запускать пример ivcmpexample.exe; библиотеки ivcmp.dll, ivcmp.lib рядом (при сборке с MinGW - ivcmp.dll; статической библиотеки .lib нет)

## Отладка

### Сохранение промежуточных данных в файлы

В исходном коде библиотеки есть макроопределение `DEBUG_FILE_OUTPUT`. По умолчанию оно закомментировано. Если его раскомментировать, библиотека начнёт сохранять промежуточные состояния кривых и другую отладочную информацию в текстовые файлы. Для визуализации кривых в папке utils есть специальный скрипт.
