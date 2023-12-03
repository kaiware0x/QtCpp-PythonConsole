#include "PythonConsoleHistory.h"

PythonConsoleHistory::PythonConsoleHistory()
{
    _it = _history.cend();
}

PythonConsoleHistory::~PythonConsoleHistory() {}

void PythonConsoleHistory::first()
{
    _it = _history.cbegin();
}

bool PythonConsoleHistory::more()
{
    return (_it != _history.cend());
}

/**
 * next switches the history pointer to the next item.
 * While searching the next item, the routine respects the search prefix set by prev().
 * @return true if the pointer was switched to a later item, false otherwise.
 */
bool PythonConsoleHistory::next()
{
    bool wentNext = false;

    // if we didn't reach history's end ...
    if (_it != _history.cend()) {
        // we go forward until we find an item matching the prefix.
        for (++_it; _it != _history.cend(); ++_it) {
            if (!_it->isEmpty() && _it->startsWith(_prefix)) {
                break;
            }
        }
        // we did a step - no matter of a matching prefix.
        wentNext = true;
    }
    return wentNext;
}

/**
 * prev switches the history pointer to the previous item.
 * The optional parameter prefix allows to search the history selectively for commands that start
 *   with a certain character sequence.
 * @param prefix - prefix string for searching backwards in history, empty string by default
 * @return true if the pointer was switched to an earlier item, false otherwise.
 */
bool PythonConsoleHistory::prev(const QString& prefix)
{
    bool wentPrev = false;

    // store prefix if it's the first history access
    if (_it == _history.cend()) {
        _prefix = prefix;
    }

    // while we didn't go back or reach history's begin ...
    while (!wentPrev && _it != _history.cbegin()) {
        // go back in history and check if item matches prefix
        // Skip empty items
        --_it;
        wentPrev = (!_it->isEmpty() && _it->startsWith(_prefix));
    }
    return wentPrev;
}

bool PythonConsoleHistory::isEmpty() const
{
    return _history.isEmpty();
}

const QString& PythonConsoleHistory::value() const
{
    return ((_it != _history.end()) ? *_it
                                    /* else */
                                    : _prefix);
}

void PythonConsoleHistory::append(const QString& item)
{
    _history.append(item);
    // reset iterator to make the next history
    //   access begin with the latest item.
    _it = _history.cend();
}

const QStringList& PythonConsoleHistory::values() const
{
    return this->_history;
}

/**
 * restart resets the history access to the latest item.
 */
void PythonConsoleHistory::restart()
{
    _it = _history.cend();
}

/**
 * markScratch stores the current end index of the history list.
 * Note: with simply remembering a start index, it does not work to nest scratch regions.
 * However, just replace the index keeping by a stack - in case this is be a concern.
 */
void PythonConsoleHistory::markScratch()
{
    _scratchBegin = _history.length();
}

/**
 * doScratch removes the tail of the history list, starting from the index marked lately.
 */
void PythonConsoleHistory::doScratch()
{
    if (_scratchBegin < _history.length()) {
        _history.erase(_history.begin() + _scratchBegin, _history.end());
        this->restart();
    }
}
