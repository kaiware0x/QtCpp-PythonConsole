#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PythonConsole/PythonConsole.h"

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

    QDockWidget   *m_py_console_dock{nullptr};
    PythonConsole *m_py_console{nullptr};
};
#endif    // MAINWINDOW_H
