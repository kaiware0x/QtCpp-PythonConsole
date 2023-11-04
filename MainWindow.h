#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PythonConsole.h"

#include <iostream>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    PythonConsole *m_py_console;
};
#endif    // MAINWINDOW_H
