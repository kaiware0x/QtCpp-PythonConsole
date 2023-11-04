#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_py_console(new PythonConsole(this))
{
    ui->setupUi(this);

    setWindowTitle("Python Console");
    setCentralWidget(m_py_console);
}

MainWindow::~MainWindow()
{
    delete ui;
}
