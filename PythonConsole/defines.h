#ifndef DEFINES_H
#define DEFINES_H

#include <QDebug>

#define VAROUT(x) "[" << #x << "," << x << "]"

#define LOG qDebug() << __FUNCTION__ << ":"
#define WARNLOG qWarning() << __FUNCTION__ << ":"

#endif    // DEFINES_H
