
#include "MainWindow.h"

#include <QApplication>
#include <iostream>

namespace py = pybind11;

int main(int argc, char *argv[])
{
    Py_UnbufferedStdioFlag = 1;    // 標準入出力のバッファリングを無効にする

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    app.exec();
    return 0;
}
