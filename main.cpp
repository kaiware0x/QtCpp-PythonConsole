#include <Python.h>

#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  // Initialize the Python interpreter
  Py_Initialize();

  // Execute some Python code
  PyRun_SimpleString("print('Hello from Python-C API!')");

  // Finalize the Python interpreter
  Py_Finalize();

  return a.exec();
}
