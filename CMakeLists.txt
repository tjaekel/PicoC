cmake_minimum_required(VERSION 3.14)

project(PicoC LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(PicoC
  PicoC.rc
  main.cpp
  mssleep.cpp
  clibrary.c
  commands.c
  expression.c
  lex.c
  library_unix.c
  math_library.c
  parse.c
  picoc.c
  picoc_heap.c
  platform.c
  platform_unix.c
  table.c
  type.c
  variable.c
  mssleep.h
  picoc.h
  picoc_commands.h
  platform.h
)

target_link_libraries(PicoC Qt${QT_VERSION_MAJOR}::Core)

install(TARGETS PicoC
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
