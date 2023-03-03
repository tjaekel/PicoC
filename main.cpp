#include <QCoreApplication>
#include <iostream>

#include "picoc.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName("PicoC");

    pico_c_main_interactive(argc, argv);

    return a.exec();
}
