Импорт проекта:
1.	Установить Visual Studio со стандартом языка C++ 20 и Git
2.	Установить Qt Creater 6.8.0 или новее
3.	Открыть Visual Studio, зайти в расширения и установить Qt Visual Studio Tools и Qt Vs CMake Tools.
4.	Открыть проект в Visual Studio клонирование репозитория (https://github.com/Dimanter/AFE_VS_test.git) (Вставить ссылку)
5.	После открытия проекта в обозревателе решений дважды нажать на AFE_VS_test.sln
6.	Импорт проекта завершён.

7.	Если возникли ошибки при запуске, зайти в свойства проекта выполняем следующие действия:
Добавить Каталоги VC++:

 Каталоги исполняемых файлов:
  ..\qt\creator\6.8.0\msvc2019_64\lib
  ..\qt\creator\6.8.0\msvc2019_64\lib\cmake\Qt6SerialPort
  $(ExecutablePath)

Добавить путь к Qt и Qt модуль serialport:
  Qt Project Settings -> Qt Installation: ..\qt\creator\6.8.0\msvc2019_64
  Qt Project Settings -> Qt Modules: core;gui;widgets;serialport
  
Добавить дополнительные включаемые файлы Qt:
 C/C++ -> Общие -> Дополнительные каталоги включаемых файлов:
  ..\AFE_VS_test\vcpkg\poco-1.13.2\Foundation\include
  ..\qt\creator\6.8.0\msvc2019_64\include\QtSerialPort
  %(AdditionalIncludeDirectories)
  
 C/C++ -> Процессор -> Определения процессора:
  $(Qt_DEFINES_)
  %(PreprocessorDefinitions)
  QT_SERIALPORT_LIB

 C/C++ -> Язык -> Стандарт языка С:
  Стандарт ISO C17 (2018) (/std:c17)  (или новее)

 Компоновщик -> Система -> Подсистема:
  Windows (/SUBSYSTEM:WINDOWS)
