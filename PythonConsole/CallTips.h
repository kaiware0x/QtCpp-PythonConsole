
#ifndef GUI_CALLTIPS_H
#define GUI_CALLTIPS_H

#include "IncludePybind11.h"

#include <QListWidget>

class QPlainTextEdit;

class CallTip {
public:
    enum Type { Unknown, Module, Class, Method, Member, Property };
    CallTip() : type(Unknown) {}
    QString name;
    QString description;
    QString parameter;
    Type    type;
};

/**
 * @author Werner Mayer
 */
class CallTipsList : public QListWidget {
    Q_OBJECT

public:
    /// Construction
    CallTipsList(QPlainTextEdit *parent);
    /// Destruction
    ~CallTipsList() override;

    void keyboardSearch(const QString &wordPrefix) override;
    void showTips(const QString &);
    void validateCursor();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private Q_SLOTS:
    void callTipItemActivated(QListWidgetItem *item);

private:
    QString                extractContext(const QString &) const;
    QMap<QString, CallTip> extractTips(const QString &) const;
    void    extractTipsFromObject(py::object &obj, const pybind11::list &list, QMap<QString, CallTip> &tips) const;
    void    extractTipsFromProperties(py::object &obj, QMap<QString, CallTip> &tips) const;
    QString stripWhiteSpace(const QString &) const;

private:
    QPlainTextEdit *textEdit;
    int             cursorPos;
    mutable bool    validObject;
    bool            doCallCompletion;
    QList<int>      hideKeys;
    QList<int>      compKeys;
};

#endif    // GUI_CALLTIPS_H
