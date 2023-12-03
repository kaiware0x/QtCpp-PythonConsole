#ifndef PYTHONCONSOLE_H
#define PYTHONCONSOLE_H

#include "PythonConsole_global.h"
#include "TextEdit.h"

class PYTHON_CONSOLE_EXPORT PythonConsole : public TextEdit {
public:
    enum Prompt { Complete = 0, Incomplete = 1, Flush = 2, Special = 3 };

    PythonConsole(QWidget* parent = nullptr);
    ~PythonConsole();

private:
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;

    void        overrideCursor(const QString& txt);
    void        insertPrompt();
    QTextCursor inputBegin() const;
    void        runSource(const QString& line);
    void        appendOutput(const QString& output, int state);
    void        appendOutput(const std::string& output, const int output_state);

    //    void setInputFormat();
    //    void setOutputFormat();
    //    void setErrorFormat();

Q_SIGNALS:
    void pendingSource();

private:
    struct PythonConsoleP* d;
};

#endif    // PYTHONCONSOLE_H
