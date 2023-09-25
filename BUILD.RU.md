# Как собрать Scantailor Experimental под Windows

## Необходимый софт

Указаны версии, с которыми выполнялась сборка релиза 0.2023.09.19:

1. **Visual Studio С++ Native Desktop**

   Сайт: https://visualstudio.microsoft.com/

   Файл: Visual Studio Community 2022

2. **CMake**

   Сайт: https://cmake.org/

   Файл: cmake-3.27.4-windows-x86_64.msi

3. **Git**

   Сайт: https://git-scm.com/

   Файл: Git-2.42.0.2-64-bit.exe

4. **Perl**

   Сайт: https://strawberryperl.com/

   Файл: strawberry-perl-5.32.1.1-64bit.msi

5. **NSIS**

   Сайт: http://nsis.sourceforge.net/

   Файл: nsis-3.09-setup.exe

6. **Eigen3**

   Сайт: https://eigen.tuxfamily.org/

   Файл: eigen-3.4.0.tar.bz2

7. **Boost**

   Сайт: https://www.boost.org/

   Файл: boost_1_83_0.7z

8. **Jom**

   Сайт: https://wiki.qt.io/Jom

   Файл: jom_1_1_4.zip

9. **Qt5**

   Cайт: https://www.qt.io/

   Файл: qt-everywhere-src-5.12.12.tar.xz

10. **zlib**

    Сайт: https://zlib.net/

    Файл: zlib-1.3.tar.xz

11. **libjpeg-turbo**

    Сайт: https://libjpeg-turbo.org/

    Файл: libjpeg-turbo-3.0.0.tar.gz

12. **libpng**

    Сайт: http://www.libpng.org/

    Файл: libpng-1.6.40.tar.xz

13. **libtiff**

    Сайт: http://www.libtiff.org/

    Файл: tiff-4.6.0.tar.xz

14. **OpenCL**

    Сайт: https://www.khronos.org/

    Файл: OpenCL-SDK-v2023.04.17-Source.tar.gz

## Установка софта

Установить **Visual Studio**, **CMake**, **Git**, **Perl**, **NSIS**, как обычно, по подсказкам инсталлятора.

## Сборка зависимостей

Сборка зависимостей производится только один раз. Все последующие сборки программы, и все сборки других версий программы используют готовые результаты этого этапа.

1. **Папка сборки**

   Создать папку сборки. В ее полном пути не должно быть пробелов. Далее будет использоваться путь "D:\Prog". Все зависимости будут лежать тут каждая в своей папке:

   ~~~ text
   D:\
   └─ Prog\
      ├─ boost_1_83_0\
      ├─ eigen-3.4.0\
      ├─ libjpeg-turbo-3.0.0\
      ├─ libpng-1.6.40\
      ├─ OpenCL-2023.04.17\
      ├─ qt-5.12.12\
      ├─ scantailor-experimental\
      ├─ tiff-4.6.0\
      └─ zlib-1.3\
   ~~~

2. **Eigen3**

   Сборка не требуется.

   Распаковать архив в свою папку, без лишних уровней вложенности. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ eigen-3.4.0\
         ├─ .gitlab\
         ├─ bench\
         ├─ blas\
         ...
   ~~~

   Сборка завершена.

3. **Boost**

   Сборка производится "на месте", и занимает некоторое время.

   Распаковать архив в свою папку, без лишних уровней вложенности Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ boost_1_83_0\
         ├─ boost\
         ├─ doc\
         ├─ libs\
         ...
         ├─ bootstrap.bat
         ...
   ~~~

   Запустить bootstrap.bat. Появится файл b2.exe.

   Запустить b2.exe.  Появятся папки bin.v2 и stage.

   Сборка завершена.

4. **Qt5**

    Начиная с Qt, и для всех библиотек далее, под хранение исходников и под сборку лучше выделить отдельные папки внутри родительской - \src и \build. По завершении сборки для экономии места их можно будет удалить. Сборка Qt идет долго, и требует много свободного места.

   Создать в папке \qt-5.12.12 папку для исходного кода \src. Распаковать в нее архив, без лишних уровней вложенности. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ qt-5.12.12\
         └─ src\
            ├─ coin\
            ├─ gnuwin32\
            ...
            ├─ qtbase\
            ...
            ├─ configure.bat
            ...
   ~~~

   Создать в папке \qt-5.12.12 папку \build для сборки.

   В папке build создать файл build.cmd следующего содержания:

   ~~~ text
   @EM Set paths
   SET PATH=%CD%\qtbase\bin;%CD%\gnuwin32\bin;%PATH%

   @REM Congigure Qt
   ..\src\configure -prefix %CD%\.. -opensource -confirm-license -platform win32-msvc -release -mp -opengl desktop -nomake examples -nomake tests

   @REM Build packages: Core Gui Widgets Xml Network OpenGL
   ..\jom\jom module-qtbase

   @REM Build packages: LinguistTools
   ..\jom\jom module-qttools

   @REM Build additional image formats
   ..\jom\jom module-qtimagefo

   @REM Install packages: Core Gui Widgets Xml Network OpenGL
   ..\jom\jom module-qtbase-install_subtargets

   @REM Install packages: LinguistTools
   ..\jom\jom module-qttools-install_subtargets

   @REM Install additional image formats
   ..\jom\jom module-qtimageformats-install_subtargets
   ~~~

   Создать в папке библиотеки \qt-5.12.12 папку \jom, и распаковать в нее утилиту jom.exe. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ qt-5.12.12\
         ├─ build\
         │  └─ build.bat
         ├─ jom\
         │  └─ jom.exe
         └─ src\
            ├─ coin\
            ├─ gnuwin32\
            ├─ qtbase\
            ...
   ~~~

   В меню Пуск выбрать ярлык "x64 Native Tools Command Prompt for VS 2022". В открывшейся консоли разработчика перейти в папку сборки Qt и запустить командный файл:

   ~~~ text
   D:
   CD D:\Prog\qt-5.12.12\build\
   build.cmd
   ~~~

   По завершении получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ qt-5.12.12\
         ├─ bin\
         ├─ build\
         ├─ doc\
         ├─ include\
         ├─ jom\
         ├─ lib\
         ├─ mkspecs\
         ├─ phrasebooks\
         ├─ plugins\
         ├─ qml\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

5. **zlib**

   Далее все библиотеки собираются вручную в два этапа. Сначала в CMake-GUI генерируется проект для Visual Studio, потом Visual Studio выполняет сборку.

   Распаковать архив в папку \src внутри \zlib-1.3, без лишних уровней вложенности.

   Создать рядом папку \build. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ zlib-1.3\
         ├─ build\
         └─ src\
            ├─ amiga\
            ├─ contrib\
            ├─ doc\
            ... 
            ├─ CMakeLists.txt
            ...
   ~~~

   Из меню Пуск запустить CMake-GUI.

   Нажать кнопку "Browse Source", выбрать папку с исходным кодом - "D:\Prog\zlib-1.3\src" (здесь лежит файл проекта CMake - CMakeLists.txt).

   Нажать кнопку "Browse Build", выбрать папку для сборки - "D:\Prog\zlib-1.3\build" (здесь будет создан проект Visual Studio, и будут лежать все временные файлы).

   Нажать кнопку "Add Entry", добавить переменную "CMAKE_INSTALL_PREFIX", типа "PATH", и, щелкнув кнопку "...", указать значение "D:\Prog\zlib-1.3" (сюда будет установлена готовая собранная библиотека).

   Нажать кнопку "Configure", согласиться с выбором генератора "Visual Studio 17 2022", нажать "Finish". В протоколе должно появиться сообщение "Configuring done".
   В папке \build появится файл CMakeCache.txt, где сохранены все выбранные настройки.

   Нажать кнопку "Generate". В протоколе должно появиться сообщение "Generating done". В папке \build появится готовый проект Visual Studio.

   Нажать кнопку "Open Project". Сгенерированный проект откроется в Visual Studio.

   В Visual Studio переключить конфигурацию решения на "Release".

   В обозревателе решений из контекстного меню проекта "INSTALL" выбрать пункт "Собрать". Библиотека будет собрана и установлена. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ zlib-1.3\
         ├─ bin\
         │  └─ zlib.dll
         ├─ build\
         ├─ include\
         │  └─ *.h
         ├─ lib\
         │  └─ *.lib
         ├─ share\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

6. **libjpeg-turbo**

   Сборка полностью аналогична сборке zlib.

   Распаковать архив в папку \src внутри \libjpeg-turbo-3.0.0, без лишних уровней вложенности.

   Создать рядом папку \build. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ libjpeg-turbo-3.0.0\
         ├─ build\
         └─ src\
            ├─ cmakescripts\
            ├─ doc\
            ├─ fuzz\
            ... 
            ├─ CMakeLists.txt
            ...
   ~~~

   Запустить CMake-GUI.

   Нажать "Browse Source", выбрать "D:\Prog\libjpeg-turbo-3.0.0\src".

   Нажать "Browse Build", выбрать "D:\Prog\libjpeg-turbo-3.0.0\build".

   Нажать "Add Entry", добавить "CMAKE_INSTALL_PREFIX", типа "PATH", выбрать  "D:\Prog\libjpeg-turbo-3.0.0".

   Выполнить "Configure".

   Выполнить "Generate".

   Нажать "Open Project".

   В Visual Studio переключить конфигурацию решения на "Release".

   Собрать проект "INSTALL". Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ libjpeg-turbo-3.0.0\
         ├─ bin\
         │  ├─ jpeg62.dll
         │  ...
         ├─ build\
         ├─ include\
         │  └─ *.h
         ├─ lib\
         │  └─ *.lib
         ├─ share\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

7. **libpng**

   Сборка похожа на сборку zlib и libjpeg, но эта библиотека зависит от zlib, и эту зависимость надо указать дополнительно, при настройке CMake.

   Распаковать архив в папку \src.

   Создать папку \build. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ libpng-1.6.40\
         ├─ build\
         └─ src\
            ├─ arm\
            ├─ ci\
            ├─ contrib\
            ... 
            ├─ CMakeLists.txt
            ...
   ~~~

   Запустить CMake-GUI.

   Нажать "Browse Source", выбрать "D:\Prog\libpng-1.6.40\src".

   Нажать "Browse Build", выбрать "D:\Prog\libpng-1.6.40\build".

   Нажать "Add Entry", добавить "CMAKE_PREFIX_PATH", типа "PATH", выбрать  "D:\Prog\zlib-1.3". Это путь, где CMake ищет нужные зависимости. Если его не задать, при конфигурации произойдет ошибка, с сообщением "Could NOT find ZLIB".

   Нажать "Add Entry", добавить "CMAKE_INSTALL_PREFIX", типа "PATH", выбрать  "D:\Prog\libpng-1.6.40".

   Выполнить "Configure".

   Выполнить "Generate".

   Нажать "Open Project".

   В Visual Studio переключить конфигурацию на "Release".

   Собрать проект "INSTALL". Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ libpng-1.6.40\
         ├─ bin\
         │  ├─ libpng16.dll
         │  ...
         ├─ build\
         ├─ include\
         │  └─ *.h
         ├─ lib\
         │  └─ *.lib
         ├─ share\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

8. **libtiff**

   Сборка аналогична libpng, но эта библиотека зависит от и zlib и от libjpeg, при настройке CMake надо указать два дополнительных пути поиска.

   Распаковать архив в папку \src.

   Создать папку \build. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ tiff-4.6.0\
         ├─ build\
         └─ src\
            ├─ build\
            ├─ cmake\
            ├─ config\
            ... 
            ├─ CMakeLists.txt
            ...
   ~~~

   Запустить CMake-GUI.

   Нажать "Browse Source", выбрать "D:\Prog\tiff-4.6.0\src".

   Нажать "Browse Build", выбрать "D:\Prog\tiff-4.6.0\build".

   Нажать "Add Entry", добавить "CMAKE_PREFIX_PATH", типа "PATH", указать пути к библиотекам zlib и libjpeg, через точку с запятой "D:/Prog/zlib-1.3;D:/Prog/libjpeg-turbo-3.0.0".

   Нажать "Add Entry", добавить "CMAKE_INSTALL_PREFIX", типа "PATH", выбрать  "D:\Prog\tiff-4.6.0".

   Выполнить "Configure".

   Выполнить "Generate".

   Нажать "Open Project".

   В Visual Studio переключить конфигурацию на "Release".

   Собрать проект "INSTALL". Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ tiff-4.6.0\
         ├─ bin\
         │  ├─ tiff.dll
         │  ...
         ├─ build\
         ├─ include\
         │  └─ *.h
         ├─ lib\
         │  └─ *.lib
         ├─ share\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

9. **OpenCL**

   OpenCL зависит еще от нескольких библиотек, но проект CMake для него настроен так, что в процессе конфигурации загрузка этих зависимостей из интернета произойдет автоматически, так что сборка аналогична сборке zlib. Загрузка зависимостей занимает некоторое время.

   Распаковать архив в папку \src.

   Создать папку \build. Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ OpenCL-2023.04.17\
         ├─ build\
         └─ src\
            ├─ .github\
            ├─ .gitlab\
            ├─ .reuse\
            ... 
            ├─ CMakeLists.txt
            ...
   ~~~

   Запустить CMake-GUI.

   Нажать "Browse Source", выбрать "D:\Prog\OpenCL-2023.04.17\src".

   Нажать "Browse Build", выбрать "D:\Prog\OpenCL-2023.04.17\build".

   Нажать "Add Entry", добавить "CMAKE_INSTALL_PREFIX", типа "PATH", выбрать  "D:\Prog\OpenCL-2023.04.17".

   Выполнить "Configure" (дождаться окончания загрузки зависимостей).

   Выполнить "Generate".

   Нажать "Open Project".

   В Visual Studio переключить конфигурацию на "Release".

   Собрать проект "INSTALL". Получится структура папок:

   ~~~ text
   D:\
   └─ Prog\
      └─ OpenCL-2023.04.17\
         ├─ bin\
         ├─ build\
         ├─ include\
         ├─ lib\
         ├─ share\
         └─ src\
   ~~~

   Папки src и build можно удалить.

   Сборка завершена.

## Сборка Scantailor Experimental

Сборка программы аналогична сборке библиотек с зависимостями, только вместо установки делается упаковка.

Распаковать архив в папку \src.

Создать папку \build. Получится структура папок:

~~~ text
D:\
└─ Prog\
   └─ scantailor-experimental\
      ├─ build\
      └─ src\
         ├─ cmake\
         ├─ packaging\
         ├─ src\
         ... 
         ├─ CMakeLists.txt
         ...
~~~

Запустить CMake-GUI.

Нажать "Browse Source", выбрать "D:\Prog\scantailor-experimental\src".

Нажать "Browse Build", выбрать "D:\Prog\scantailor-experimental\build".

Нажать "Add Entry", добавить "CMAKE_PREFIX_PATH", типа "PATH", указать пути ко всем библиотекам через точку с запятой "D:/Prog/boost_1_83_0;D:/Prog/eigen-3.4.0;D:/Prog/libjpeg-turbo-3.0.0;D:/Prog/libpng-1.6.40;D:/Prog/qt-5.12.12;D:/Prog/tiff-4.6.0;D:/Prog/zlib-1.3;D:/Prog/OpenCL-2023.04.17".

Выполнить "Configure".

Выполнить "Generate".

Нажать "Open Project".

В Visual Studio переключить конфигурацию на "Release".

Собрать проект "PACKAGE".

В папке \build\staging находится собранная программа, а в папке \build лежит готовый инсталлятор:

~~~ text
D:\
└─ Prog\
   └─ scantailor-experimental\
      └─ build\
         ├─ staging\
         │  ├─ scantailor-experimental.exe
         │  ├─ scantailor-experimental-cli.exe
         │  ...
         └─ scantailor-experimental-64bit-install.exe
~~~

Сборка завершена.
