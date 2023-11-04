#include "PythonConsole.h"

#include "PyStdoutRedirector.h"

#include <QCoreApplication>
#include <QDockWidget>
#include <QTextBlock>
#include <iostream>

namespace {
static constexpr QChar promptEnd(QLatin1Char(' '));    //< char for detecting prompt end

inline int promptLength(const QString &lineStr)
{
    return lineStr.indexOf(promptEnd) + 1;
}

/**
 * @brief
 * @param lineStr
 * @return lineStr から ">>> "などを取り除いたテキスト.
 */
inline QString stripPromptFrom(const QString &lineStr)
{
    return lineStr.mid(promptLength(lineStr));
}

/**
 * cursorBeyond checks if cursor is at a valid position to accept keyEvents.
 * @param cursor - cursor to check
 * @param limit  - cursor that marks the begin of the input region
 * @param shift  - offset for shifting the limit for non-selection cursors [default: 0]
 * @return true if a keyEvent is ok at cursor's position, false otherwise
 */
inline bool cursorBeyond(const QTextCursor &cursor, const QTextCursor &limit, int shift = 0)
{
    int pos = limit.position();
    if (cursor.hasSelection()) {
        return (cursor.selectionStart() >= pos && cursor.selectionEnd() >= pos);
    }
    else {
        return cursor.position() >= (pos + shift);
    }
}

std::string pyobj2str(py::object pyobj)
{
    try {
        return py::str(pyobj).cast<std::string>();
    }
    catch (py::error_already_set ex) {
        std::cerr << ex.what() << std::endl;
        return std::string{};
    }
}
}    // namespace

// class Logger {
//     PythonConsole *console = nullptr;

// public:
//     Logger(PythonConsole *console_) : console(console_) {}
//     void write(std::string str) { console->appendPlainText(QString::fromStdString(str)); }
//     void flush() { std::cout << std::flush; }
// };

// PYBIND11_EMBEDDED_MODULE(embeded, module)
//{
//     auto logger = py::class_<Logger>(module, "Logger");
//     logger.def(py::init<PythonConsole *>());
//     logger.def("write", &Logger::write);
//     logger.def("flush", &Logger::flush);
// }

struct PythonConsoleP {
    enum Output { Error = 20, Message = 21 };
    enum CopyType { Normal, History, Command };
    CopyType  type;
    PyObject *_stdoutPy, *_stderrPy, *_stdinPy, *_stdin;
    //    CallTipsList         *callTipsList;
    PythonConsoleHistory  history;
    QString               output, error, info, historyFile;
    QStringList           statements;
    bool                  interactive;
    QMap<QString, QColor> colormap;    // Color map
    PythonConsoleP()
    {
        type = Normal;
        //        callTipsList                             = nullptr;
        interactive                              = false;
        historyFile                              = QString::fromUtf8(std::string("PythonHistory.log").c_str());
        colormap[QLatin1String("Text")]          = Qt::black;
        colormap[QLatin1String("Bookmark")]      = Qt::cyan;
        colormap[QLatin1String("Breakpoint")]    = Qt::red;
        colormap[QLatin1String("Keyword")]       = Qt::blue;
        colormap[QLatin1String("Comment")]       = QColor(0, 170, 0);
        colormap[QLatin1String("Block comment")] = QColor(160, 160, 164);
        colormap[QLatin1String("Number")]        = Qt::blue;
        colormap[QLatin1String("String")]        = Qt::red;
        colormap[QLatin1String("Character")]     = Qt::red;
        colormap[QLatin1String("Class name")]    = QColor(255, 170, 0);
        colormap[QLatin1String("Define name")]   = QColor(255, 170, 0);
        colormap[QLatin1String("Operator")]      = QColor(160, 160, 164);
        colormap[QLatin1String("Python output")] = QColor(170, 170, 127);
        colormap[QLatin1String("Python error")]  = Qt::red;
    }
};

///---------------------------------------------------------
///
///---------------------------------------------------------

/**
 * @brief PythonConsole::PythonConsole
 * @param parent
 */
PythonConsole::PythonConsole(QWidget *parent)
    : TextEdit(parent), m_interpreter(std::make_unique<py::scoped_interpreter>()), m_sys(py::module::import("sys"))
{
    //    auto emb = py::module_::import("embeded");
    //    m_sys.attr("stdout") = m_pylog_redirector;
    //    m_sys.attr("stderr") = m_pylog_redirector;
    //    m_sys.attr("stdout") = emb.attr("Logger")(this);
    //    m_sys.attr("stderr") = emb.attr("Logger")(this);
    //    m_sys.attr("stderr") = Logger(this);

    const char *version  = PyUnicode_AsUTF8(PySys_GetObject("version"));
    const char *platform = PyUnicode_AsUTF8(PySys_GetObject("platform"));
    const auto  signature =
        QString::fromLatin1("Python %1 on %2\n"
                            "Type 'help', 'copyright', 'credits' or 'license' for more information.\n")
            .arg(QString::fromLatin1(version), QString::fromLatin1(platform));
    appendOutput(signature.toStdString());

    insertPrompt();
    //    QTextCursor cursor = this->textCursor();
    //    cursor.insertText(">>> ");
    //    cursor.movePosition(QTextCursor::End);
    //    setTextCursor(cursor);
}

void PythonConsole::keyPressEvent(QKeyEvent *e)
{
    QTextCursor       currentCursor     = this->textCursor();
    const QTextCursor inputBeginCursor  = this->inputBegin();
    const QTextBlock  inputBlock        = inputBeginCursor.block();    //< get the last paragraph's text
    const QString     inputBlockText    = inputBlock.text();
    const QString     inputStriptedText = stripPromptFrom(inputBlockText);
    //    if (this->_sourceDrain && !this->_sourceDrain->isEmpty()) {
    //        inputStrg = inputLine.mid(this->_sourceDrain->length());
    //    }

    bool restartHistory{true};

    if (!cursorBeyond(currentCursor, inputBeginCursor)) {
        /// カーソル位置がTextEditの末尾にいない場合の処理.
        /**
         * The cursor is placed not on the input line (or within the prompt string)
         * So we handle key input as follows:
         *   - don't allow changing previous lines.
         *   - allow full movement (no prompt restriction)
         *   - allow copying content (Ctrl+C)
         *   - "escape" to end of input line
         */
        switch (e->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Escape:
            case Qt::Key_Backspace:
                this->moveCursor(QTextCursor::End);
                break;

            default:
                if (e->text().isEmpty() || e->matches(QKeySequence::Copy) || e->matches(QKeySequence::SelectAll)) {
                    TextEdit::keyPressEvent(e);
                }
                else if (!e->text().isEmpty()
                         && (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::ShiftModifier)) {
                    this->moveCursor(QTextCursor::End);
                    TextEdit::keyPressEvent(e);
                }
                break;
        }
    }
    else {
        switch (e->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter: {
                this->moveCursor(QTextCursor::End);
                m_history.append(inputStriptedText);
                runSource(inputStriptedText);    /// commit input string
                TextEdit::keyPressEvent(e);
                this->moveCursor(QTextCursor::End);
                insertPrompt();
                break;
            }
            case Qt::Key_Backspace: {
                if (cursorBeyond(currentCursor, inputBeginCursor, 1)) {
                    TextEdit::keyPressEvent(e);
                }
                break;
            }

            case Qt::Key_Left: {
                if (currentCursor > inputBeginCursor) {
                    TextEdit::keyPressEvent(e);
                }
                restartHistory = false;
                break;
            }

            case Qt::Key_Right: {
                TextEdit::keyPressEvent(e);
                restartHistory = false;
                break;
            }

            case Qt::Key_Up: {
                // if possible, move back in history
                if (m_history.prev(inputStriptedText)) {
                    overrideCursor(m_history.value());
                }
                restartHistory = false;
                break;
            }
            case Qt::Key_Down: {
                // if possible, move forward in history
                if (m_history.next()) {
                    overrideCursor(m_history.value());
                }
                restartHistory = false;
                break;
            }

            case Qt::Key_Home: {
                QTextCursor::MoveMode mode = (e->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor
                                                                                  /* else */
                                                                                  : QTextCursor::MoveAnchor;
                currentCursor.setPosition(inputBeginCursor.position(), mode);
                setTextCursor(currentCursor);
                ensureCursorVisible();
                break;
            }
            default: {
                TextEdit::keyPressEvent(e);
                break;
            }
        }

        /// disable history restart if input line changed
        restartHistory &= (inputStriptedText != inputBlockText);
    }

    /// any cursor move resets the history to its latest item.
    if (restartHistory) {
        m_history.restart();
    }
}

void PythonConsole::showEvent(QShowEvent *e)
{
    TextEdit::showEvent(e);
    // set also the text cursor to the edit field
    setFocus();
}

void PythonConsole::overrideCursor(const QString &txt)
{
    // Go to the last line and the fourth position, right after the prompt
    QTextCursor cursor      = this->inputBegin();
    int         blockLength = this->textCursor().block().text().length();

    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, blockLength);    //<< select text to override
    cursor.removeSelectedText();
    cursor.insertText(txt);
    // move cursor to the end
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

void PythonConsole::insertPrompt()
{
    insertPlainText(">>> ");
    moveCursor(QTextCursor::End);
}

/**
 * @brief 行の先頭の >>> などの直後のカーソル位置を取得する.
 * @return コンソール末尾行の ">>> " 直後のカーソル位置オブジェクト
 */
QTextCursor PythonConsole::inputBegin() const
{
    // construct cursor at begin of input line ...
    QTextCursor inputLineBegin(this->textCursor());
    inputLineBegin.movePosition(QTextCursor::End);
    inputLineBegin.movePosition(QTextCursor::StartOfBlock);
    // ... and move cursor right beyond the prompt.
    int prompt_len = promptLength(inputLineBegin.block().text());
    if (this->_sourceDrain && !this->_sourceDrain->isEmpty()) {
        prompt_len = this->_sourceDrain->length();
    }
    inputLineBegin.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, prompt_len);
    return inputLineBegin;
}

void PythonConsole::runSource(QString line)
{
    using namespace py::literals;

    //    py::object locals  = py::dict();
    //    auto       globals = py::globals();
    PyStdErrOutStreamRedirect redirector{};

    try {
        auto ret = py::eval(line.toUtf8().constData());
        if (ret.is_none()) {
            std::cout << "eval: return object is None.\n";
            redirector.stdoutRefresh();
            redirector.stderrRefresh();
            throw std::exception{};
        }
        auto str = pyobj2str(ret);
        appendOutput(str);
        std::cout << "eval: " << redirector.stdoutString() << std::endl;
        return;
        //        auto global_vars = py::globals();
        //        if (global_vars.contains(line.toUtf8().constData())) {
        //            auto val     = global_vars[line.toUtf8().constData()];
        //            auto val_str = py::str(val).cast<std::string>();
        //            appendPlainText(QString::fromStdString(val_str));
        //            return ;
        //        }
    }
    catch (...) {
        //        appendPlainText(ex.what());
    }

    try {
        //        std::cout << "debug: " << line.toUtf8().data() << std::endl;
        py::exec(line.toUtf8().data());
        appendOutput(redirector.stdoutString());
        std::cout << "exec: " << redirector.stdoutString() << std::endl;
    }
    catch (py::error_already_set ex) {
        appendOutput(ex.what(), true);
    }

    //    for (auto &&[key, val] : py::globals()) {
    //        std::cout << "key: " << py::str(key).cast<std::string>();
    //        std::cout << ", value: " << py::str(val).cast<std::string>();
    //        std::cout << std::endl;
    //    }
    //    std::cout << std::endl << std::endl;
}

void PythonConsole::printPrompt(Prompt mode)
{
    // Append the prompt string
    QTextCursor cursor = textCursor();

    if (mode != PythonConsole::Special) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::End);
        QTextBlock block = cursor.block();

        // Python's print command appends a trailing '\n' to the system output.
        // In this case, however, we should not add a new text block. We force
        // the current block to be normal text (user state = 0) to be highlighted
        // correctly and append the '>>> ' or '... ' to this block.
        if (block.length() > 1)
            cursor.insertBlock(cursor.blockFormat(), cursor.charFormat());
        else
            block.setUserState(0);

        switch (mode) {
            case PythonConsole::Incomplete:
                cursor.insertText(QString::fromLatin1("... "));
                break;
            case PythonConsole::Complete:
                cursor.insertText(QString::fromLatin1(">>> "));
                break;
            default:
                break;
        }
        cursor.endEditBlock();
    }
    // move cursor to the end
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

void PythonConsole::printStatement(const QString &cmd) {}

void PythonConsole::appendOutput(const QString &output, int state)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    int pos = cursor.position() + 1;

    // delay rehighlighting
    cursor.beginEditBlock();
    appendPlainText(output);

    QTextBlock block = this->document()->findBlock(pos);
    while (block.isValid()) {
        block.setUserState(state);
        block = block.next();
    }
    cursor.endEditBlock();    // start highlightiong
}

void PythonConsole::appendOutput(const std::string &output, const bool is_error)
{
    if (output.empty()) {
        return;
    }

    if (is_error) {
        setErrorFormat();
    }
    else {
        setOutputFormat();
    }

    appendPlainText(QString::fromStdString(output));

    setInputFormat();
}

void PythonConsole::setInputFormat()
{
    auto cursor = textCursor();
    // 新しい文字色を設定する QTextCharFormat を作成
    QTextCharFormat format;
    format.setForeground(Qt::black);
    format.setFontItalic(false);

    // 現在の選択範囲またはカーソル位置に新しい書式を適用
    cursor.mergeCharFormat(format);

    setTextCursor(cursor);
}

void PythonConsole::setOutputFormat()
{
    auto cursor = textCursor();
    // 新しい文字色を設定する QTextCharFormat を作成
    QTextCharFormat format;
    format.setForeground(Qt::darkGray);
    format.setFontItalic(false);

    // 現在の選択範囲またはカーソル位置に新しい書式を適用
    cursor.mergeCharFormat(format);

    setTextCursor(cursor);
}

void PythonConsole::setErrorFormat()
{
    auto cursor = textCursor();
    // 新しい文字色を設定する QTextCharFormat を作成
    QTextCharFormat format;
    format.setForeground(Qt::red);
    format.setFontItalic(true);

    // 現在の選択範囲またはカーソル位置に新しい書式を適用
    cursor.mergeCharFormat(format);

    setTextCursor(cursor);
}

///---------------------------------------------------------
///
///---------------------------------------------------------

PythonConsoleHighlighter::PythonConsoleHighlighter(QObject *parent) : PythonSyntaxHighlighter(parent) {}

PythonConsoleHighlighter::~PythonConsoleHighlighter() {}

void PythonConsoleHighlighter::highlightBlock(const QString &text)
{
    const int ErrorOutput   = (int)PythonConsoleP::Error;
    const int MessageOutput = (int)PythonConsoleP::Message;

    // Get user state to re-highlight the blocks in the appropriate format
    int stateOfPara = currentBlockState();

    switch (stateOfPara) {
        case ErrorOutput: {
            // Error output
            QTextCharFormat errorFormat;
            errorFormat.setForeground(color(QLatin1String("Python error")));
            errorFormat.setFontItalic(true);
            setFormat(0, text.length(), errorFormat);
        } break;
        case MessageOutput: {
            // Normal output
            QTextCharFormat outputFormat;
            outputFormat.setForeground(color(QLatin1String("Python output")));
            setFormat(0, text.length(), outputFormat);
        } break;
        default: {
            PythonSyntaxHighlighter::highlightBlock(text);
        } break;
    }
}

void PythonConsoleHighlighter::colorChanged(const QString &type, const QColor &col)
{
    Q_UNUSED(type);
    Q_UNUSED(col);
}
