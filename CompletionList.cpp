#include "CompletionList.h"

CompletionList::CompletionList(QPlainTextEdit* parent) : QListWidget(parent), textEdit(parent)
{
    // make the user assume that the widget is active
    QPalette pal = parent->palette();
    pal.setColor(QPalette::Inactive, QPalette::Highlight, pal.color(QPalette::Active, QPalette::Highlight));
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::HighlightedText));
    parent->setPalette(pal);

    connect(this, &CompletionList::itemActivated, this, &CompletionList::completionItem);
}

CompletionList::~CompletionList() {}

void CompletionList::findCurrentWord(const QString& wordPrefix)
{
    for (int i = 0; i < count(); ++i) {
        QString text = item(i)->text();
        if (text.startsWith(wordPrefix)) {
            setCurrentRow(i);
            return;
        }
    }

    if (currentItem()) currentItem()->setSelected(false);
}

/**
 * Get all incoming events of the text edit and redirect some of them, like key up and
 * down, mouse press events, ... to the widget itself.
 */
bool CompletionList::eventFilter(QObject* watched, QEvent* event)
{
    if (isVisible() && watched == textEdit->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) hide();
    }
    else if (isVisible() && watched == textEdit) {
        if (event->type() == QEvent::KeyPress) {
            auto ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
                keyPressEvent(ke);
                return true;
            }
            else if (ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown) {
                keyPressEvent(ke);
                return true;
            }
            else if (ke->key() == Qt::Key_Escape) {
                hide();
                return true;
            }
            else if (ke->key() == Qt::Key_Space) {
                hide();
                return false;
            }
            else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                Q_EMIT itemActivated(currentItem());
                return true;
            }
        }
        else if (event->type() == QEvent::FocusOut) {
            if (!hasFocus()) hide();
        }
    }

    return QListWidget::eventFilter(watched, event);
}

/**
 * If an item was chosen (either by clicking or pressing enter) the rest of the word is completed.
 * The listbox is closed without destroying it.
 */
void CompletionList::completionItem(QListWidgetItem* item)
{
    hide();
    QString     text   = item->text();
    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    textEdit->ensureCursorVisible();
}
