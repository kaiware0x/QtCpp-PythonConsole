#ifndef PYTHONSYNTAXHIGHLIGHTER_H
#define PYTHONSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighterP;
class PythonSyntaxHighlighterP;
class TextEditor;

/**
 * Abstract Syntax highlighter.
 * @author Werner Mayer
 */
class SyntaxHighlighter : public QSyntaxHighlighter {
public:
    SyntaxHighlighter(QObject* parent);
    virtual ~SyntaxHighlighter();

    int maximumUserState() const;

    //    void   setColor(const QString& type, const QColor& col);
    QColor color(const QString& type);

protected:
    virtual void colorChanged(const QString& type, const QColor& col);

protected:
    enum TColor {
        Text         = 0,
        Comment      = 1,
        BlockComment = 2,
        Number       = 3,
        String       = 4,
        Keyword      = 5,
        Classname    = 6,
        Defname      = 7,
        Operator     = 8,
        Output       = 9,
        Error        = 10
    };

    QColor colorByType(TColor type);

private:
    SyntaxHighlighterP* d;
};

/**
 * Syntax highlighter for Python.
 * \author Werner Mayer
 */
class PythonSyntaxHighlighter : public SyntaxHighlighter {
public:
    explicit PythonSyntaxHighlighter(QObject* parent);
    virtual ~PythonSyntaxHighlighter();

    void highlightBlock(const QString& text);

private:
    PythonSyntaxHighlighterP* d;
};

#endif    // PYTHONSYNTAXHIGHLIGHTER_H
