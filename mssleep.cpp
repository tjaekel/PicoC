#include <QLibrary>
#include <QThread>

#include "msSleep.h"

void msSleep(unsigned long ms)
{
    QThread::msleep(ms);
}
