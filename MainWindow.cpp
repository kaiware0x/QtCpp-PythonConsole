#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_py_console_dock(new QDockWidget("Python Console", this))
    , m_py_console(new PythonConsole(this))
{
    ui->setupUi(this);

    setWindowTitle("Main Window");

    m_py_console_dock->setWidget(m_py_console);
    addDockWidget(Qt::BottomDockWidgetArea, m_py_console_dock);
}

MainWindow::~MainWindow()
{
    delete ui;
}
