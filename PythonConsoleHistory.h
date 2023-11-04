#ifndef PYTHONCONSOLEHISTORY_H
#define PYTHONCONSOLEHISTORY_H

#include <QStringList>

class PythonConsoleHistory {
public:
    PythonConsoleHistory();
    ~PythonConsoleHistory();

    void               first();
    bool               more();
    bool               next();
    bool               prev(const QString& prefix = QString());
    bool               isEmpty() const;
    const QString&     value() const;
    void               append(const QString& inputLine);
    const QStringList& values() const;
    void               restart();
    void               markScratch();
    void               doScratch();

private:
    QStringList                _history;
    QStringList::ConstIterator _it;
    int                        _scratchBegin{0};
    QString                    _prefix;
};

#endif    // PYTHONCONSOLEHISTORY_H
