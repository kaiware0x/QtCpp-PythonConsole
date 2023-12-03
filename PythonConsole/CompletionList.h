#ifndef COMPLETIONLIST_H
#define COMPLETIONLIST_H

#include <QListWidget>
#include <QPlainTextEdit>

/**
 * The CompletionList class provides a list box that pops up in a text edit if the user has pressed
 * an accelerator to complete the current word they are typing in.
 * @author Werner Mayer
 */
class CompletionList : public QListWidget {
    Q_OBJECT

public:
    /// Construction
    explicit CompletionList(QPlainTextEdit *parent);
    /// Destruction
    ~CompletionList() override;

    void findCurrentWord(const QString &);

protected:
    bool eventFilter(QObject *, QEvent *) override;

private Q_SLOTS:
    void completionItem(QListWidgetItem *item);

private:
    QPlainTextEdit *textEdit;
};

#endif    // COMPLETIONLIST_H
