#include "CallTips.h"
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextCursor>
#include <QToolTip>
#include <iostream>
// #include "BitmapFactory.h"
#include "BitmapFactory.h"
#include "IncludePybind11.h"

#include "defines.h"
#include "pyutil.h"

/**
 * template class Temporary.
 * Allows variable changes limited to a scope.
 */
template <typename TYPE>
class Temporary {
public:
    Temporary(TYPE& var, const TYPE tmpVal) : _var(var), _saveVal(var) { var = tmpVal; }

    ~Temporary() { _var = _saveVal; }

private:
    TYPE& _var;
    TYPE  _saveVal;
};

CallTipsList::CallTipsList(QPlainTextEdit* parent)
    : QListWidget(parent), textEdit(parent), cursorPos(0), validObject(true), doCallCompletion(false)
{
    // make the user assume that the widget is active
    QPalette pal = parent->palette();
    pal.setColor(QPalette::Inactive, QPalette::Highlight, pal.color(QPalette::Active, QPalette::Highlight));
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::HighlightedText));
    parent->setPalette(pal);

    connect(this, &QListWidget::itemActivated, this, &CallTipsList::callTipItemActivated);

    hideKeys.append(Qt::Key_Space);
    hideKeys.append(Qt::Key_Exclam);
    hideKeys.append(Qt::Key_QuoteDbl);
    hideKeys.append(Qt::Key_NumberSign);
    hideKeys.append(Qt::Key_Dollar);
    hideKeys.append(Qt::Key_Percent);
    hideKeys.append(Qt::Key_Ampersand);
    hideKeys.append(Qt::Key_Apostrophe);
    hideKeys.append(Qt::Key_Asterisk);
    hideKeys.append(Qt::Key_Plus);
    hideKeys.append(Qt::Key_Comma);
    hideKeys.append(Qt::Key_Minus);
    hideKeys.append(Qt::Key_Period);
    hideKeys.append(Qt::Key_Slash);
    hideKeys.append(Qt::Key_Colon);
    hideKeys.append(Qt::Key_Semicolon);
    hideKeys.append(Qt::Key_Less);
    hideKeys.append(Qt::Key_Equal);
    hideKeys.append(Qt::Key_Greater);
    hideKeys.append(Qt::Key_Question);
    hideKeys.append(Qt::Key_At);
    hideKeys.append(Qt::Key_Backslash);

    compKeys.append(Qt::Key_ParenLeft);
    compKeys.append(Qt::Key_ParenRight);
    compKeys.append(Qt::Key_BracketLeft);
    compKeys.append(Qt::Key_BracketRight);
    compKeys.append(Qt::Key_BraceLeft);
    compKeys.append(Qt::Key_BraceRight);

    setFrameStyle(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setLineWidth(2);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

CallTipsList::~CallTipsList() {}

void CallTipsList::keyboardSearch(const QString& wordPrefix)
{
    // first search for the item that matches perfectly
    for (int i = 0; i < count(); ++i) {
        QString text = item(i)->text();
        if (text.startsWith(wordPrefix)) {
            setCurrentRow(i);
            return;
        }
    }

    // now do a case insensitive comparison
    for (int i = 0; i < count(); ++i) {
        QString text = item(i)->text();
        if (text.startsWith(wordPrefix, Qt::CaseInsensitive)) {
            setCurrentRow(i);
            return;
        }
    }

    if (currentItem()) currentItem()->setSelected(false);
}

void CallTipsList::validateCursor()
{
    QTextCursor cursor     = textEdit->textCursor();
    int         currentPos = cursor.position();
    if (currentPos < this->cursorPos) {
        hide();
    }
    else {
        cursor.setPosition(this->cursorPos);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        QString word = cursor.selectedText();
        if (!word.isEmpty()) {
            // the following text might be an operator, brackets, ...
            const QChar underscore = QLatin1Char('_');
            const QChar ch         = word.at(0);
            if (!ch.isLetterOrNumber() && ch != underscore) word.clear();
        }
        if (currentPos > this->cursorPos + word.length()) {
            hide();
        }
        else if (!word.isEmpty()) {
            // If the word is empty we should not allow to do a search,
            // otherwise we may select the next item which is not okay in this
            // context. This might happen if e.g. Shift is pressed.
            keyboardSearch(word);
        }
    }
}

QString CallTipsList::extractContext(const QString& line) const
{
    int len   = line.size();
    int index = len - 1;
    for (int i = 0; i < len; i++) {
        int        pos = len - 1 - i;
        const char ch  = line.at(pos).toLatin1();
        if ((ch >= 48 && ch <= 57) ||        // Numbers
            (ch >= 65 && ch <= 90) ||        // Uppercase letters
            (ch >= 97 && ch <= 122) ||       // Lowercase letters
            (ch == '.') || (ch == '_') ||    // dot or underscore
            (ch == ' ') || (ch == '\t'))     // whitespace (between dot and text)
            index = pos;
        else
            break;
    }

    return line.mid(index);
}

QMap<QString, CallTip> CallTipsList::extractTips(const QString& context) const
{
    py::gil_scoped_acquire lock;
    QMap<QString, CallTip> tips;
    if (context.isEmpty()) return tips;

    try {
        auto global_dict = py::globals();
        pyutil::printDict(global_dict);

        // this is used to filter out input of the form "1."
        QStringList   words           = context.split(QLatin1Char('.'));
        const QString module_name     = words.front();
        const auto    module_name_std = std::string(module_name.toLatin1());
        words.pop_front();

        // unknown object
        if (!global_dict.contains(module_name_std)) {
            return tips;
        }

        pyutil::printType(global_dict[module_name_std.c_str()]);
        pyutil::printAttr(global_dict[module_name_std.c_str()]);

        auto obj = py::eval(context.toLatin1().constData());
        if (obj.is_none()) {
            return tips;
        }

        // These are the attributes from the type object
        extractTipsFromObject(obj, pyutil::getAllAttr(obj), tips);
    }
    catch (py::error_already_set e) {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "... error" << std::endl;
    }

    return tips;
}

void CallTipsList::extractTipsFromObject(py::object& obj, const py::list& list, QMap<QString, CallTip>& tips) const
{
    for (auto&& item : list) {
        try {
            auto attrname_opt = pyutil::toStdStr(item);
            if (!attrname_opt.has_value()) {
                continue;
            }
            const std::string attrname_std = attrname_opt.value();

            py::object attr = pyutil::getAttr(obj, attrname_std);
            if (attr.is_none()) {
                WARNLOG << "Python attriute is None." << VAROUT(attrname_std);
                continue;
            }

            CallTip tip;
            QString attrname_qstr = QString::fromLatin1(attrname_std.c_str());
            tip.name              = attrname_qstr;

            if (pyutil::isCallable(attr)) {
                if (pyutil::isClassInstance(attr)) {
                    tip.type = CallTip::Class;
                }
                else {
                    tip.type = CallTip::Method;
                }
            }
            else if (PyModule_Check(attr.ptr())) {
                tip.type = CallTip::Module;
            }
            else {
                tip.type = CallTip::Member;
            }

            if (attrname_qstr == QLatin1String("__doc__") && pyutil::isString(attr)) {
                py::object help = attr;
                if (pyutil::isString(help)) {
                    py::str doc(help);
                    QString longdoc = QString::fromStdString(doc.cast<std::string>());
                    int     pos     = longdoc.indexOf(QLatin1Char('\n'));
                    pos             = qMin(pos, 70);
                    if (pos < 0) pos = qMin(longdoc.length(), 70);
                    tip.description = stripWhiteSpace(longdoc);
                    tip.parameter   = longdoc.left(pos);
                }
            }
            else if (pyutil::hasAttr(attr, "__doc__")) {
                py::object help = pyutil::getAttr(attr, "__doc__");
                if (pyutil::isString(help)) {
                    py::str doc(help);
                    QString longdoc = QString::fromStdString(doc.cast<std::string>());
                    int     pos     = longdoc.indexOf(QLatin1Char('\n'));
                    pos             = qMin(pos, 70);
                    if (pos < 0) pos = qMin(longdoc.length(), 70);
                    tip.description = stripWhiteSpace(longdoc);
                    tip.parameter   = longdoc.left(pos);
                }
            }

            // Do not override existing items
            if (!tips.contains(attrname_qstr)) {
                tips[attrname_qstr] = tip;
            }
        }
        catch (py::error_already_set& e) {
            // Just clear the Python exception
            WARNLOG << e.what();
        }
    }
}

void CallTipsList::extractTipsFromProperties(py::object& obj, QMap<QString, CallTip>& tips) const
{
    //     auto                    cont      = static_cast<App::PropertyContainerPy*>(obj.ptr());
    //     App::PropertyContainer* container = cont->getPropertyContainerPtr();
    // Make sure that the C++ object is alive
    //     if (!container) return;
    //     std::map<std::string, App::Property*> Map;
    //     container->getPropertyMap(Map);

    //    for (const auto& It : Map) {
    //        CallTip tip;
    //        QString str     = QString::fromLatin1(It.first.c_str());
    //        tip.name        = str;
    //        tip.type        = CallTip::Property;
    //        QString longdoc = QString::fromUtf8(container->getPropertyDocumentation(It.second));
    //        // a point, mesh or shape property
    //        if (It.second->isDerivedFrom(Base::Type::fromName("App::PropertyComplexGeoData"))) {
    //            Py::Object data(It.second->getPyObject(), true);
    //            if (data.hasAttr("__doc__")) {
    //                Py::Object help = data.getAttr("__doc__");
    //                if (help.isString()) {
    //                    Py::String doc(help);
    //                    longdoc = QString::fromUtf8(doc.as_string().c_str());
    //                }
    //            }
    //        }
    //        if (!longdoc.isEmpty()) {
    //            int pos = longdoc.indexOf(QLatin1Char('\n'));
    //            pos     = qMin(pos, 70);
    //            if (pos < 0) pos = qMin(longdoc.length(), 70);
    //            tip.description = stripWhiteSpace(longdoc);
    //            tip.parameter   = longdoc.left(pos);
    //        }
    //        tips[str] = tip;
    //    }
}

void CallTipsList::showTips(const QString& line)
{
    // search only once
    static QPixmap type_module_icon = BitmapFactory().pixmap("ClassBrowser/type_module.svg");
    static QPixmap type_class_icon  = BitmapFactory().pixmap("ClassBrowser/type_class.svg");
    static QPixmap method_icon      = BitmapFactory().pixmap("ClassBrowser/method.svg");
    static QPixmap member_icon      = BitmapFactory().pixmap("ClassBrowser/member.svg");
    static QPixmap property_icon    = BitmapFactory().pixmap("ClassBrowser/property.svg");

    // object is in error state
    static const char* const forbidden_xpm[] = {"8 8 3 1",  ". c None", "# c #ff0000", "a c #ffffff",
                                                "..####..", ".######.", "########",    "#aaaaaa#",
                                                "#aaaaaa#", "########", ".######.",    "..####.."};
    static QPixmap           forbidden_icon(forbidden_xpm);
    static QPixmap           forbidden_type_module_icon =
        BitmapFactory().merge(type_module_icon, forbidden_icon, BitmapFactoryInst::BottomLeft);
    static QPixmap forbidden_type_class_icon =
        BitmapFactory().merge(type_class_icon, forbidden_icon, BitmapFactoryInst::BottomLeft);
    static QPixmap forbidden_method_icon =
        BitmapFactory().merge(method_icon, forbidden_icon, BitmapFactoryInst::BottomLeft);
    static QPixmap forbidden_member_icon =
        BitmapFactory().merge(member_icon, forbidden_icon, BitmapFactoryInst::BottomLeft);
    static QPixmap forbidden_property_icon =
        BitmapFactory().merge(property_icon, forbidden_icon, BitmapFactoryInst::BottomLeft);

    this->validObject           = true;
    QString context             = extractContext(line);
    context                     = context.simplified();
    QMap<QString, CallTip> tips = extractTips(context);
    clear();

    /// TODO: コメントアウトを解除して動くようにする.

    for (auto&& key : tips.keys()) {
        const auto tip = tips.value(key);

        addItem(key);
        QListWidgetItem* item = this->item(this->count() - 1);
        item->setData(Qt::ToolTipRole, QVariant(tip.description));
        item->setData(Qt::UserRole, QVariant::fromValue(tip));    //< store full CallTip data
        switch (tip.type) {
            case CallTip::Module: {
                item->setIcon((this->validObject ? type_module_icon : forbidden_type_module_icon));
            } break;
            case CallTip::Class: {
                item->setIcon((this->validObject ? type_class_icon : forbidden_type_class_icon));
            } break;
            case CallTip::Method: {
                item->setIcon((this->validObject ? method_icon : forbidden_method_icon));
            } break;
            case CallTip::Member: {
                item->setIcon((this->validObject ? member_icon : forbidden_member_icon));
            } break;
            case CallTip::Property: {
                item->setIcon((this->validObject ? property_icon : forbidden_property_icon));
            } break;
            default:
                break;
        }
    }

    if (count() == 0) return;    // nothing found

    // get the minimum width and height of the box
    int h = 0;
    int w = 0;
    for (int i = 0; i < count(); ++i) {
        QRect r = visualItemRect(item(i));
        w       = qMax(w, r.width());
        h += r.height();
    }

    // Add an offset
    w += 2 * frameWidth();
    h += 2 * frameWidth();

    // get the start position of the word prefix
    QTextCursor cursor = textEdit->textCursor();
    this->cursorPos    = cursor.position();
    QRect rect         = textEdit->cursorRect(cursor);
    int   posX         = rect.x();
    int   posY         = rect.y();
    int   boxH         = h;

    // Decide whether to show downstairs or upstairs
    if (posY > textEdit->viewport()->height() / 2) {
        h = qMin(qMin(h, posY), 250);
        if (h < boxH) w += textEdit->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        setGeometry(posX, posY - h, w, h);
    }
    else {
        h = qMin(qMin(h, textEdit->viewport()->height() - fontMetrics().height() - posY), 250);
        if (h < boxH) w += textEdit->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        setGeometry(posX, posY + fontMetrics().height(), w, h);
    }

    setCurrentRow(0);
    show();
}

void CallTipsList::showEvent(QShowEvent* e)
{
    QListWidget::showEvent(e);
    // install this object to filter timer events for the tooltip label
    qApp->installEventFilter(this);
}

void CallTipsList::hideEvent(QHideEvent* e)
{
    QListWidget::hideEvent(e);
    qApp->removeEventFilter(this);
}

/**
 * Get all incoming events of the text edit and redirect some of them, like key up and
 * down, mouse press events, ... to the widget itself.
 */
bool CallTipsList::eventFilter(QObject* watched, QEvent* event)
{
    // This is a trick to avoid to hide the tooltip window after the defined time span
    // of 10 seconds. We just filter out all timer events to keep the label visible.
    if (watched->inherits("QLabel")) {
        auto label = qobject_cast<QLabel*>(watched);
        // Ignore the timer events to prevent from being closed
        if (label->windowFlags() & Qt::ToolTip && event->type() == QEvent::Timer) return true;
    }
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
            else if ((ke->key() == Qt::Key_Minus) && (ke->modifiers() & Qt::ShiftModifier)) {
                // do nothing here, but this check is needed to ignore underscore
                // which in Qt 4.8 gives Key_Minus instead of Key_Underscore
            }
            else if (this->hideKeys.indexOf(ke->key()) > -1) {
                Q_EMIT itemActivated(currentItem());
                return false;
            }
            else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                Q_EMIT itemActivated(currentItem());
                return true;
            }
            else if (ke->key() == Qt::Key_Tab) {
                // enable call completion for activating items
                Temporary<bool> tmp(this->doCallCompletion, true);    //< previous state restored on scope exit
                Q_EMIT itemActivated(currentItem());
                return true;
            }
            else if (this->compKeys.indexOf(ke->key()) > -1) {
                Q_EMIT itemActivated(currentItem());
                return false;
            }
            else if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_Control || ke->key() == Qt::Key_Meta
                     || ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_AltGr) {
                // filter these meta keys to avoid to call keyboardSearch()
                return true;
            }
        }
        else if (event->type() == QEvent::KeyRelease) {
            auto ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_PageUp
                || ke->key() == Qt::Key_PageDown) {
                QList<QListWidgetItem*> items = selectedItems();
                if (!items.isEmpty()) {
                    QPoint  p(width(), 0);
                    QString text = items.front()->toolTip();
                    if (!text.isEmpty()) {
                        QToolTip::showText(mapToGlobal(p), text);
                    }
                    else {
                        QToolTip::showText(p, QString());
                    }
                }
                return true;
            }
        }
        else if (event->type() == QEvent::FocusOut) {
            if (!hasFocus()) hide();
        }
    }

    return QListWidget::eventFilter(watched, event);
}

void CallTipsList::callTipItemActivated(QListWidgetItem* item)
{
    hide();
    if (!item->isSelected()) return;

    QString     text   = item->text();
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(this->cursorPos);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    QString sel = cursor.selectedText();
    if (!sel.isEmpty()) {
        // in case the cursor moved too far on the right side
        const QChar underscore = QLatin1Char('_');
        const QChar ch         = sel.at(sel.length() - 1);
        if (!ch.isLetterOrNumber() && ch != underscore) cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    }
    cursor.insertText(text);

    // get CallTip from item's UserRole-data
    auto callTip = item->data(Qt::UserRole).value<CallTip>();

    // if call completion enabled and we've something callable (method or class constructor) ...
    if (this->doCallCompletion && (callTip.type == CallTip::Method || callTip.type == CallTip::Class)) {
        cursor.insertText(QLatin1String("()"));    //< just append parenthesis to identifier even inserted.

        /**
         * Try to find out if call needs arguments.
         * For this we search the description for appropriate hints ...
         */
        static QRegularExpression argumentMatcher(QRegularExpression::escape(callTip.name)
                                                  + QLatin1String("\\s*\\(\\s*\\w+.*\\)"));
        argumentMatcher.setPatternOptions(QRegularExpression::InvertedGreedinessOption);    //< set regex non-greedy!
        if (argumentMatcher.match(callTip.description).hasMatch()) {
            // if arguments are needed, we just move the cursor one left, to between the parentheses.
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
            textEdit->setTextCursor(cursor);
        }
    }
    textEdit->ensureCursorVisible();

    QRect rect = textEdit->cursorRect(cursor);
    int   posX = rect.x();
    int   posY = rect.y();

    QPoint p(posX, posY);
    p = textEdit->mapToGlobal(p);
    QToolTip::showText(p, callTip.parameter);
}

QString CallTipsList::stripWhiteSpace(const QString& str) const
{
    QString     stripped = str;
    QStringList lines    = str.split(QLatin1String("\n"));
    int         minspace = INT_MAX;
    int         line     = 0;

    for (QStringList::iterator it = lines.begin(); it != lines.end(); ++it, ++line) {
        if (it->length() > 0 && line > 0) {
            int space = 0;
            for (int i = 0; i < it->length(); i++) {
                if ((*it)[i] == QLatin1Char('\t'))
                    space++;
                else
                    break;
            }

            if (it->length() > space) minspace = std::min<int>(minspace, space);
        }
    }

    // remove all leading tabs from each line
    if (minspace > 0 && minspace < INT_MAX) {
        int         line = 0;
        QStringList strippedlines;
        for (QStringList::iterator it = lines.begin(); it != lines.end(); ++it, ++line) {
            if (line == 0 && !it->isEmpty()) {
                strippedlines << *it;
            }
            else if (it->length() > 0 && line > 0) {
                strippedlines << it->mid(minspace);
            }
        }

        stripped = strippedlines.join(QLatin1String("\n"));
    }

    return stripped;
}
