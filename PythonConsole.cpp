#include "PythonConsole.h"

#include "CallTips.h"
#include "PyStdoutRedirector.h"

#include <QCoreApplication>
#include <QDockWidget>
#include <QTextBlock>
#include <iostream>

namespace {
static const auto      PROMPT      = QString::fromLatin1(">>> ");
static const auto      PROMPT_CONT = QString::fromLatin1("... ");
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

struct PythonConsoleP {
    enum Output { Error = 20, Message = 21 };
    PythonConsoleHistory history;
    QString              info;
    CallTipsList        *callTipsList;
    PythonConsoleP() {}
};

///---------------------------------------------------------
///
///---------------------------------------------------------

/**
 * @brief PythonConsole::PythonConsole
 * @param parent
 */
PythonConsole::PythonConsole(QWidget *parent)
    : TextEdit(parent)
    , d(new PythonConsoleP)
    , m_interpreter(std::make_unique<py::scoped_interpreter>())
    , m_sys(py::module::import("sys"))
    , pythonSyntax(new PythonConsoleHighlighter(this))
{
    pythonSyntax->setDocument(this->document());

    // create the window for call tips
    d->callTipsList = new CallTipsList(this);
    d->callTipsList->setFrameStyle(QFrame::Box);
    d->callTipsList->setFrameShadow(QFrame::Raised);
    d->callTipsList->setLineWidth(2);
    installEventFilter(d->callTipsList);
    viewport()->installEventFilter(d->callTipsList);
    d->callTipsList->setSelectionMode(QAbstractItemView::SingleSelection);
    d->callTipsList->hide();

    const char *version  = PyUnicode_AsUTF8(PySys_GetObject("version"));
    const char *platform = PyUnicode_AsUTF8(PySys_GetObject("platform"));

    d->info = QString::fromLatin1("Python %1 on %2\n"
                                  "Type 'help', 'copyright', 'credits' or 'license' for more information.")
                  .arg(QString::fromLatin1(version), QString::fromLatin1(platform));

    appendOutput(d->info + "\n", PythonConsoleP::Message);

    textCursor().block().setUserState(0);    /// 1行目にSyntaxHighlightが効くようにする.
    insertPrompt();
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

            case Qt::Key_Period: {
                // In Qt 4.8 there is a strange behaviour because when pressing ":"
                // then key is also set to 'Period' instead of 'Colon'. So we have
                // to make sure we only handle the period.
                if (e->text() == QLatin1String(".")) {
                    // analyse context and show available call tips
                    int contextLength = currentCursor.position() - inputBeginCursor.position();
                    TextEdit::keyPressEvent(e);
                    d->callTipsList->showTips(inputStriptedText.left(contextLength));
                }
                else {
                    TextEdit::keyPressEvent(e);
                }
            } break;
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
    insertPlainText(PROMPT);
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

void PythonConsole::runSource(const QString &line)
{
    m_history.append(line);

    PyStdErrOutStreamRedirect redirector;
    py::gil_scoped_acquire    lock;

    try {
        auto ret = py::eval(line.toUtf8().constData());
        if (ret.is_none()) {
            std::cout << "eval: return object is None.\n";
            redirector.stdoutRefresh();
            redirector.stderrRefresh();
            throw std::exception{};
        }
        appendOutput(pyobj2str(ret), PythonConsoleP::Message);
    }
    catch (...) {
        /// if py::eval() was failed.
        try {
            py::exec(line.toUtf8().data());
            appendOutput(redirector.stdoutString(), PythonConsoleP::Message);
            std::cout << "exec: " << redirector.stdoutString() << std::endl;
        }
        catch (py::error_already_set ex) {
            appendOutput(QString(ex.what()), PythonConsoleP::Error);
        }
    }
}

void PythonConsole::appendOutput(const QString &output, int state)
{
    if (output.isEmpty()) {
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    int pos = cursor.position() + 1;

    cursor.beginEditBlock();    // delay rehighlighting

    appendPlainText(output);

    QTextBlock block = this->document()->findBlock(pos);
    while (block.isValid()) {
        block.setUserState(state);
        block = block.next();
    }
    cursor.endEditBlock();    // start highlightiong
}

/**
 * @brief PythonConsole::appendOutput
 * @param output
 * @param output_state : PythonConsoleP::Output
 */
void PythonConsole::appendOutput(const std::string &output, const int output_state)
{
    return appendOutput(QString::fromStdString(output), output_state);
}

// void PythonConsole::setInputFormat()
//{
//     auto cursor = textCursor();
//     // 新しい文字色を設定する QTextCharFormat を作成
//     QTextCharFormat format;
//     format.setForeground(Qt::black);
//     format.setFontItalic(false);

//    // 現在の選択範囲またはカーソル位置に新しい書式を適用
//    cursor.mergeCharFormat(format);

//    setTextCursor(cursor);
//}

// void PythonConsole::setOutputFormat()
//{
//     auto cursor = textCursor();
//     // 新しい文字色を設定する QTextCharFormat を作成
//     QTextCharFormat format;
//     format.setForeground(Qt::darkGray);
//     format.setFontItalic(false);

//    // 現在の選択範囲またはカーソル位置に新しい書式を適用
//    cursor.mergeCharFormat(format);

//    setTextCursor(cursor);
//}

// void PythonConsole::setErrorFormat()
//{
//     auto cursor = textCursor();
//     // 新しい文字色を設定する QTextCharFormat を作成
//     QTextCharFormat format;
//     format.setForeground(Qt::red);
//     format.setFontItalic(true);

//    // 現在の選択範囲またはカーソル位置に新しい書式を適用
//    cursor.mergeCharFormat(format);

//    setTextCursor(cursor);
//}

///---------------------------------------------------------
///
///---------------------------------------------------------

PythonConsoleHighlighter::PythonConsoleHighlighter(QObject *parent) : PythonSyntaxHighlighter(parent) {}

PythonConsoleHighlighter::~PythonConsoleHighlighter() {}

void PythonConsoleHighlighter::highlightBlock(const QString &text)
{
    // Get user state to re-highlight the blocks in the appropriate format
    int stateOfPara = currentBlockState();

    switch (stateOfPara) {
        case PythonConsoleP::Error: {
            // Error output
            QTextCharFormat errorFormat;
            errorFormat.setForeground(color(QLatin1String("Python error")));
            errorFormat.setFontItalic(true);
            setFormat(0, text.length(), errorFormat);
        } break;
        case PythonConsoleP::Message: {
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
