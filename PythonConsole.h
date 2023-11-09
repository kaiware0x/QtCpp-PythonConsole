#ifndef PYTHONCONSOLE_H
#define PYTHONCONSOLE_H

/// QtとPythonでマクロ slots の名前が競合しているので回避
/// https://stackoverflow.com/questions/23068700/embedding-python3-in-qt-5

#include "IncludePybind11.h"
#include "PythonConsoleHistory.h"
#include "PythonSyntaxHighlighter.h"
#include "TextEdit.h"

class PythonConsoleHighlighter;

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

    std::unique_ptr<py::scoped_interpreter> m_interpreter;
    py::module_                             m_sys;

    QString* _sourceDrain = nullptr;
    QString  _historyFile;

    PythonConsoleHighlighter* pythonSyntax;

    PythonConsoleHistory m_history;

    int m_font_size{10};
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
    void colorChanged(const QString& type, const QColor& col) override {}
};

#endif    // PYTHONCONSOLE_H
