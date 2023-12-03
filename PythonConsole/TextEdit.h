#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>
#include <QTextEdit>

#include "PythonConsole_global.h"

/**
 * Completion is a means by which an editor automatically completes words that the user is typing.
 * For example, in a code editor, a programmer might type "sur", then Tab, and the editor will complete
 * the word the programmer was typing so that "sur" is replaced by "surnameLineEdit". This is very
 * useful for text that contains long words or variable names. The completion mechanism usually works
 * by looking at the existing text to see if any words begin with what the user has typed, and in most
 * editors completion is invoked by a special key sequence.
 *
 * TextEdit can detect a special key sequence to invoke the completion mechanism, and can handle three
 * different situations:
 * \li There are no possible completions.
 * \li There is a single possible completion.
 * \li There are two or more possible completions.
 *
 * \remark The original sources are taken from Qt Quarterly (Customizing for Completion).
 * @author Werner Mayer
 */
class CompletionList;
class PYTHON_CONSOLE_EXPORT TextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TextEdit(QWidget *parent = nullptr);
    virtual ~TextEdit();

private Q_SLOTS:
    void complete();

Q_SIGNALS:
    void showSearchBar();
    void findNext();
    void findPrevious();

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    void createListBox();

private:
    QString         wordPrefix;
    int             cursorPosition;
    CompletionList *listBox;
};

#endif    // TEXTEDIT_H
