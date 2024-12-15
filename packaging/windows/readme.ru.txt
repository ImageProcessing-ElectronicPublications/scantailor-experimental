This file is in UTF-8 encoding.

Этот документ описывает процесс сборки Scan Tailor под Windows.

Ранние версии Scan Tailor поддерживали как Visual Studio так и MinGW.
С какого-то момента времени, поддержка MinGW была убрана, с целью снизить
усилия по сопровождению кода. Кроме того Scan Tailor начал использовать
некоторые возможности стандарта C++17, из-за чего версии Visual Studio
до 2017 перестали официально поддерживаться.


                            Скачиваем необходимый софт

Первым делом, нам понадобится нижеследующий софт.  Если не указано обратного,
всегда берите последние стабильные версии.

1. Visual Studio 2017 Community for Windows Desktop или новее.
   Сайт: https://visualstudio.microsoft.com/
2. CMake >= 2.8.9
   Сайт: http://www.cmake.org
3. jpeg library
   Сайт: http://www.ijg.org/
   Нам нужен файл jpegsrc.v9.tar.gz или с похожим именем.
4. zlib
   Сайт: http://www.zlib.net/
   Нам нужен файл вида zlib-x.x.x.tar.gz, где x.x.x - номер версии.
5. libpng
   Сайт: http://www.libpng.org/pub/png/libpng.html
   Нам нужен файл вида libpng-x.x.x.tar.gz, где x.x.x - номер версии.
6. libtiff
   Сайт: http://www.remotesensing.org/libtiff/
   Из-за того, что libtiff обновляется редко, а дыры в нем находят часто, лучше
   всего будет его сразу же пропатчить.  В таком случае брать его нужно отсюда:
   http://packages.debian.org/source/sid/tiff
   Там и сам libtiff и набор патчей для него.  Процесс наложения патчей описан
   далее в этом документе.  Если вы не собираетесь распространять ваши сборки
   Scan Tailor'а и не собираетесь открывать им файлы из сомнительных источников,
   тогда можете и не патчить libtiff.
7. Qt5 >= 5.3 (протестировано с Qt 5.12.12, 6.8.1)
   Cайт: http://qt-project.org/
   Скачивать можно любую бинарную версию (используемый компилятор не важен),
   или даже версию только с исходниками. В любом случае будет сделана
   специальная сборка Qt, но в первом случае надо будет собирать не так
   много вещей, как во втором.
8. Strawberry Perl (нужно для сборки Qt)
   Сайт: https://strawberryperl.com/
   При установке убедитесь, что опция "Add Perl to the PATH
   environment variable" включена.
9. Boost (протестировано с 1.86.0)
   Сайт: http://boost.org/
   Качайте boost в любом формате, при условии что вы знаете, как этот формат
   распаковывать.
10. Eigen3 (протестировано с 3.4.0)
   Сайт: http://eigen.tuxfamily.org/
11. NSIS 3.x (протестировано с 3.09)
   Сайт: http://nsis.sourceforge.net/
   Также следует скачать плагин https://nsis.sourceforge.io/NsProcess_plugin и установить в папку с nsis:
   * Папку `NsProcess.zip\Include` копируем в `c:\Program Files (x86)\NSIS\`.
   * Файл `NsProcess.zip\Plugin\nsProcess.dll` копируем в `c:\Program Files (x86)\NSIS\Plugins\x86-ansi\`.
   * Файл `NsProcess.zip\Plugin\nsProcessW.dll` копируем в `c:\Program Files (x86)\NSIS\Plugins\x86-unicode` и переименовываем в `nsProcess.dll`.

   
                                    Инструкции

1. Создать директорию сборки.  В ее полном пути не должно быть пробелов.
   Далее в документе будет предполагаться директория C:\build

2. Распаковать jpeg-{версия}, libpng, libtiff, zlib, boost, Eigen, и сам
   scantailor в директорию сборки.  В результате должна получиться примерно
   такая структура директорий:
   C:\build
     | boost_1_86_0
     | eigen-3.4.0
     | jpeg-9e
     | libpng-1.6.44
     | scantailor
     | tiff-4.7.0
     | zlib-1.3.1
   
   Если брали версию Qt без инсталлятора, распаковываем ее сюда же.
   В противном случае ставим Qt в директорию, предлагаемую инсталлятором.
   ВАЖНО: инсталлятору нужно указать, чтобы ставил также и исходники
   (Source Components).
   
   Установите также Visual Studio, Strawberry Perl.
   
   Если не знаете, чем распаковывать .tar.gz файлы, попробуйте вот этим:
   http://www.7-zip.org/
   
3. Установить Visual Studio, Strawberry Perl, NSIS и CMake.

4. Соберите зависимости (boost, jpeg, libpng, tiff, zlib, Qt)
   Вы также можете использовать vcpkg вместе с CMake для скачивания и сборки зависимостей, если не хотите делать это руками.

5. Соберите scantailor (укажите пути к собранным boost, eigen, jpeg, libpng, scantailor, tiff, zlib, Qt)