#ifndef PYTHONCONSOLE_H
#define PYTHONCONSOLE_H

/// QtとPythonでマクロ slots の名前が競合しているので回避
/// https://stackoverflow.com/questions/23068700/embedding-python3-in-qt-5
#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>    // pybind11の埋め込み機能を使用するために必要
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma pop_macro("slots")

#include "PythonConsoleHistory.h"
#include "PythonSyntaxHighlighter.h"
#include "TextEdit.h"

class Logger;

class PythonConsole : public TextEdit {
public:
    enum Prompt { Complete = 0, Incomplete = 1, Flush = 2, Special = 3 };

    PythonConsole(QWidget* parent = nullptr);

private:
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;

    void        overrideCursor(const QString& txt);
    void        insertPrompt();
    QTextCursor inputBegin() const;
    void        runSource(QString line);
    void        printPrompt(Prompt mode);
    void        printStatement(const QString& cmd);
    void        appendOutput(const QString& output, int state);
    void        appendOutput(const std::string& output, const bool is_error = false);

    void setInputFormat();
    void setOutputFormat();
    void setErrorFormat();

Q_SIGNALS:
    void pendingSource();

private:
    std::unique_ptr<py::scoped_interpreter> m_interpreter;
    py::module_                             m_sys;

    QString* _sourceDrain = nullptr;
    QString  _historyFile;

    PythonConsoleHistory m_history;
};

/**
 * Syntax highlighter for Python console.
 * @author Werner Mayer
 */
class PythonConsoleHighlighter : public PythonSyntaxHighlighter {
public:
    explicit PythonConsoleHighlighter(QObject* parent);
    ~PythonConsoleHighlighter() override;

    void highlightBlock(const QString& text) override;

protected:
    void colorChanged(const QString& type, const QColor& col) override;
};

#endif    // PYTHONCONSOLE_H
