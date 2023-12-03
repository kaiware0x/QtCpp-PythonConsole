#include "TextEdit.h"
#include "CompletionList.h"

#include <QShortcut>
#include <QTextBlock>

/**
 *  Constructs a TextEdit which is a child of 'parent'.
 */
TextEdit::TextEdit(QWidget* parent) : QPlainTextEdit(parent), cursorPosition(0), listBox(nullptr)
{
    // Note: Set the correct context to this shortcut as we may use several instances of this
    // class at a time
    auto shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QString::fromLatin1("CTRL+Space")));
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, &TextEdit::complete);

    auto shortcutFind = new QShortcut(this);
    shortcutFind->setKey(QKeySequence::Find);
    shortcutFind->setContext(Qt::WidgetShortcut);
    connect(shortcutFind, &QShortcut::activated, this, &TextEdit::showSearchBar);

    auto shortcutNext = new QShortcut(this);
    shortcutNext->setKey(QKeySequence::FindNext);
    shortcutNext->setContext(Qt::WidgetShortcut);
    connect(shortcutNext, &QShortcut::activated, this, &TextEdit::findNext);

    auto shortcutPrev = new QShortcut(this);
    shortcutPrev->setKey(QKeySequence::FindPrevious);
    shortcutPrev->setContext(Qt::WidgetShortcut);
    connect(shortcutPrev, &QShortcut::activated, this, &TextEdit::findPrevious);
}

/** Destroys the object and frees any allocated resources */
TextEdit::~TextEdit() {}

/**
 * Set the approproriate item of the completion box or hide it, if needed.
 */
void TextEdit::keyPressEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyPressEvent(e);
    // This can't be done in CompletionList::eventFilter() because we must first perform
    // the event and afterwards update the list widget
    if (listBox && listBox->isVisible()) {
        // Get the word under the cursor
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfWord);
        // the cursor has moved to outside the word prefix
        if (cursor.position() < cursorPosition - wordPrefix.length() || cursor.position() > cursorPosition) {
            listBox->hide();
            return;
        }
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        listBox->keyboardSearch(cursor.selectedText());
        cursor.clearSelection();
    }
}

/**
 * Completes the word.
 */
void TextEdit::complete()
{
    QTextBlock block = textCursor().block();
    if (!block.isValid()) return;
    int     cursorPos = textCursor().position() - block.position();
    QString para      = block.text();
    int     wordStart = cursorPos;
    while (wordStart > 0 && para[wordStart - 1].isLetterOrNumber())
        --wordStart;
    wordPrefix = para.mid(wordStart, cursorPos - wordStart);
    if (wordPrefix.isEmpty()) return;

    QStringList            list = toPlainText().split(QRegularExpression(QLatin1String("\\W+")));
    QMap<QString, QString> map;
    QStringList::Iterator  it = list.begin();
    while (it != list.end()) {
        if ((*it).startsWith(wordPrefix) && (*it).length() > wordPrefix.length()) map[(*it).toLower()] = *it;
        ++it;
    }

    if (map.count() == 1) {
        insertPlainText((*map.begin()).mid(wordPrefix.length()));
    }
    else if (map.count() > 1) {
        if (!listBox) createListBox();
        listBox->clear();
        listBox->addItems(map.values());
        listBox->setFont(QFont(font().family(), 8));

        this->cursorPosition = textCursor().position();

        // get the minimum width and height of the box
        int h = 0;
        int w = 0;
        for (int i = 0; i < listBox->count(); ++i) {
            QRect r = listBox->visualItemRect(listBox->item(i));
            w       = qMax(w, r.width());
            h += r.height();
        }

        // Add an offset
        w += 2 * listBox->frameWidth();
        h += 2 * listBox->frameWidth();

        // get the start position of the word prefix
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfWord);
        QRect rect = cursorRect(cursor);
        int   posX = rect.x();
        int   posY = rect.y();
        int   boxH = h;

        // Decide whether to show downstairs or upstairs
        if (posY > viewport()->height() / 2) {
            h = qMin(qMin(h, posY), 250);
            if (h < boxH) w += style()->pixelMetric(QStyle::PM_ScrollBarExtent);
            listBox->setGeometry(posX, posY - h, w, h);
        }
        else {
            h = qMin(qMin(h, viewport()->height() - fontMetrics().height() - posY), 250);
            if (h < boxH) w += style()->pixelMetric(QStyle::PM_ScrollBarExtent);
            listBox->setGeometry(posX, posY + fontMetrics().height(), w, h);
        }

        listBox->setCurrentRow(0);
        listBox->show();
    }
}

/**
 * Creates the listbox containing all possibilities for the completion.
 * The listbox is closed when ESC is pressed, the text edit field loses focus or a
 * mouse button was pressed.
 */
void TextEdit::createListBox()
{
    listBox = new CompletionList(this);
    listBox->setFrameStyle(QFrame::Box);
    listBox->setFrameShadow(QFrame::Raised);
    listBox->setLineWidth(2);
    installEventFilter(listBox);
    viewport()->installEventFilter(listBox);
    listBox->setSelectionMode(QAbstractItemView::SingleSelection);
    listBox->hide();
}
