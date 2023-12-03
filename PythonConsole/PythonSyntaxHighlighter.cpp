#include "PythonSyntaxHighlighter.h"

#include <QApplication>
#include <QPalette>

class SyntaxHighlighterP {
public:
    SyntaxHighlighterP()
    {
        cNormalText = qApp->palette().windowText().color();
        cComment.setRgb(0, 170, 0);
        cBlockcomment.setRgb(160, 160, 164);
        cLiteral.setRgb(255, 0, 0);
        cNumber.setRgb(0, 0, 255);
        cOperator.setRgb(160, 160, 164);
        cKeyword.setRgb(0, 0, 255);
        cClassName.setRgb(255, 170, 0);
        cDefineName.setRgb(255, 170, 0);
        cOutput.setRgb(170, 170, 127);
        cError.setRgb(255, 0, 0);

        colormap[QLatin1String("Text")]          = cNormalText;
        colormap[QLatin1String("Bookmark")]      = Qt::cyan;
        colormap[QLatin1String("Breakpoint")]    = Qt::red;
        colormap[QLatin1String("Keyword")]       = cKeyword;
        colormap[QLatin1String("Comment")]       = cComment;
        colormap[QLatin1String("Block comment")] = cBlockcomment;
        colormap[QLatin1String("Number")]        = cNumber;
        colormap[QLatin1String("String")]        = cLiteral;
        colormap[QLatin1String("Character")]     = cLiteral;
        colormap[QLatin1String("Class name")]    = cClassName;
        colormap[QLatin1String("Define name")]   = cDefineName;
        colormap[QLatin1String("Operator")]      = cOperator;
        colormap[QLatin1String("Python output")] = cOutput;
        colormap[QLatin1String("Python error")]  = cError;
    }

    QColor cNormalText, cComment, cBlockcomment, cLiteral, cNumber, cOperator, cKeyword, cClassName, cDefineName,
        cOutput, cError;

    QMap<QString, QColor> colormap;
};

/**
 * Constructs a syntax highlighter.
 */
SyntaxHighlighter::SyntaxHighlighter(QObject* parent) : QSyntaxHighlighter(parent)
{
    d = new SyntaxHighlighterP;
}

/** Destroys this object. */
SyntaxHighlighter::~SyntaxHighlighter()
{
    delete d;
}

/** Sets the color \a col to the paragraph type \a type.
 * This method is provided for convenience to specify the paragraph type
 * by its name.
 */
/*
void SyntaxHighlighter::setColor(const QString& type, const QColor& col)
{
    // Rehighlighting is very expensive, thus avoid it if this color is already set
    QColor old = color(type);
    if (!old.isValid()) {
        return;    // no such type
    }
    if (old == col) {
        return;
    }

    if (type == QLatin1String("Text"))
        d->cNormalText = col;
    else if (type == QLatin1String("Comment"))
        d->cComment = col;
    else if (type == QLatin1String("Block comment"))
        d->cBlockcomment = col;
    else if (type == QLatin1String("Number"))
        d->cNumber = col;
    else if (type == QLatin1String("String"))
        d->cLiteral = col;
    else if (type == QLatin1String("Keyword"))
        d->cKeyword = col;
    else if (type == QLatin1String("Class name"))
        d->cClassName = col;
    else if (type == QLatin1String("Define name"))
        d->cDefineName = col;
    else if (type == QLatin1String("Operator"))
        d->cOperator = col;
    else if (type == QLatin1String("Python output"))
        d->cOutput = col;
    else if (type == QLatin1String("Python error"))
        d->cError = col;
    colorChanged(type, col);
}
*/

QColor SyntaxHighlighter::color(const QString& type)
{
    return d->colormap.value(type, QColor());

    /*
        if (type == QLatin1String("Text"))
            return d->cNormalText;
        else if (type == QLatin1String("Comment"))
            return d->cComment;
        else if (type == QLatin1String("Block comment"))
            return d->cBlockcomment;
        else if (type == QLatin1String("Number"))
            return d->cNumber;
        else if (type == QLatin1String("String"))
            return d->cLiteral;
        else if (type == QLatin1String("Keyword"))
            return d->cKeyword;
        else if (type == QLatin1String("Class name"))
            return d->cClassName;
        else if (type == QLatin1String("Define name"))
            return d->cDefineName;
        else if (type == QLatin1String("Operator"))
            return d->cOperator;
        else if (type == QLatin1String("Python output"))
            return d->cOutput;
        else if (type == QLatin1String("Python error"))
            return d->cError;
        else
            return QColor();    // not found
        */
}

QColor SyntaxHighlighter::colorByType(SyntaxHighlighter::TColor type)
{
    if (type == SyntaxHighlighter::Text)
        return d->cNormalText;
    else if (type == SyntaxHighlighter::Comment)
        return d->cComment;
    else if (type == SyntaxHighlighter::BlockComment)
        return d->cBlockcomment;
    else if (type == SyntaxHighlighter::Number)
        return d->cNumber;
    else if (type == SyntaxHighlighter::String)
        return d->cLiteral;
    else if (type == SyntaxHighlighter::Keyword)
        return d->cKeyword;
    else if (type == SyntaxHighlighter::Classname)
        return d->cClassName;
    else if (type == SyntaxHighlighter::Defname)
        return d->cDefineName;
    else if (type == SyntaxHighlighter::Operator)
        return d->cOperator;
    else if (type == SyntaxHighlighter::Output)
        return d->cOutput;
    else if (type == SyntaxHighlighter::Error)
        return d->cError;
    else
        return QColor();    // not found
}

void SyntaxHighlighter::colorChanged(const QString& type, const QColor& col)
{
    Q_UNUSED(type);
    Q_UNUSED(col);
    rehighlight();
}

int SyntaxHighlighter::maximumUserState() const
{
    return 8;
}

///---------------------------------------------------------
///
///---------------------------------------------------------

class PythonSyntaxHighlighterP {
public:
    PythonSyntaxHighlighterP()
    {
        keywords << QLatin1String("and") << QLatin1String("as") << QLatin1String("assert") << QLatin1String("break")
                 << QLatin1String("class") << QLatin1String("continue") << QLatin1String("def") << QLatin1String("del")
                 << QLatin1String("elif") << QLatin1String("else") << QLatin1String("except") << QLatin1String("exec")
                 << QLatin1String("False") << QLatin1String("finally") << QLatin1String("for") << QLatin1String("from")
                 << QLatin1String("global") << QLatin1String("if") << QLatin1String("import") << QLatin1String("in")
                 << QLatin1String("is") << QLatin1String("lambda") << QLatin1String("None") << QLatin1String("nonlocal")
                 << QLatin1String("not") << QLatin1String("or") << QLatin1String("pass") << QLatin1String("print")
                 << QLatin1String("raise") << QLatin1String("return") << QLatin1String("True") << QLatin1String("try")
                 << QLatin1String("while") << QLatin1String("with") << QLatin1String("yield");
    }

    QStringList keywords;
};

/**
 * Constructs a Python syntax highlighter.
 */
PythonSyntaxHighlighter::PythonSyntaxHighlighter(QObject* parent) : SyntaxHighlighter(parent)
{
    d = new PythonSyntaxHighlighterP;
}

/** Destroys this object. */
PythonSyntaxHighlighter::~PythonSyntaxHighlighter()
{
    delete d;
}

/**
 * Detects all kinds of text to highlight them in the correct color.
 */
void PythonSyntaxHighlighter::highlightBlock(const QString& text)
{
    int   i = 0;
    QChar prev, ch;

    constexpr int Standard      = 0;    // Standard text
    constexpr int Digit         = 1;    // Digits
    constexpr int Comment       = 2;    // Comment begins with #
    constexpr int Literal1      = 3;    // String literal beginning with "
    constexpr int Literal2      = 4;    // Other string literal beginning with '
    constexpr int Blockcomment1 = 5;    // Block comments beginning and ending with """
    constexpr int Blockcomment2 = 6;    // Other block comments beginning and ending with '''
    constexpr int ClassName     = 7;    // Text after the keyword class
    constexpr int DefineName    = 8;    // Text after the keyword def

    int endStateOfLastPara = previousBlockState();
    if (endStateOfLastPara < 0 || endStateOfLastPara > maximumUserState()) {
        endStateOfLastPara = Standard;
    }

    while (i < text.length()) {
        ch = text.at(i);

        switch (endStateOfLastPara) {
            case Standard: {
                switch (ch.unicode()) {
                    case '#': {
                        // begin a comment
                        setFormat(i, 1, this->colorByType(SyntaxHighlighter::Comment));
                        endStateOfLastPara = Comment;
                    } break;
                    case '"': {
                        // Begin either string literal or block comment
                        if ((i >= 2) && text.at(i - 1) == QLatin1Char('"') && text.at(i - 2) == QLatin1Char('"')) {
                            setFormat(i - 2, 3, this->colorByType(SyntaxHighlighter::BlockComment));
                            endStateOfLastPara = Blockcomment1;
                        }
                        else {
                            setFormat(i, 1, this->colorByType(SyntaxHighlighter::String));
                            endStateOfLastPara = Literal1;
                        }
                    } break;
                    case '\'': {
                        // Begin either string literal or block comment
                        if ((i >= 2) && text.at(i - 1) == QLatin1Char('\'') && text.at(i - 2) == QLatin1Char('\'')) {
                            setFormat(i - 2, 3, this->colorByType(SyntaxHighlighter::BlockComment));
                            endStateOfLastPara = Blockcomment2;
                        }
                        else {
                            setFormat(i, 1, this->colorByType(SyntaxHighlighter::String));
                            endStateOfLastPara = Literal2;
                        }
                    } break;
                    case ' ':
                    case '\t': {
                        // ignore whitespaces
                    } break;
                    case '(':
                    case ')':
                    case '[':
                    case ']':
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case ':':
                    case '%':
                    case '^':
                    case '~':
                    case '!':
                    case '=':
                    case '<':
                    case '>':    // possibly two characters
                    {
                        setFormat(i, 1, this->colorByType(SyntaxHighlighter::Operator));
                        endStateOfLastPara = Standard;
                    } break;
                    default: {
                        // Check for normal text
                        if (ch.isLetter() || ch == QLatin1Char('_')) {
                            QString buffer;
                            int     j = i;
                            while (ch.isLetterOrNumber() || ch == QLatin1Char('_')) {
                                buffer += ch;
                                ++j;
                                if (j >= text.length()) break;    // end of text
                                ch = text.at(j);
                            }

                            if (d->keywords.contains(buffer) != 0) {
                                if (buffer == QLatin1String("def"))
                                    endStateOfLastPara = DefineName;
                                else if (buffer == QLatin1String("class"))
                                    endStateOfLastPara = ClassName;

                                QTextCharFormat keywordFormat;
                                keywordFormat.setForeground(this->colorByType(SyntaxHighlighter::Keyword));
                                keywordFormat.setFontWeight(QFont::Bold);
                                setFormat(i, buffer.length(), keywordFormat);
                            }
                            else {
                                setFormat(i, buffer.length(), this->colorByType(SyntaxHighlighter::Text));
                            }

                            // increment i
                            if (!buffer.isEmpty()) i = j - 1;
                        }
                        // this is the beginning of a number
                        else if (ch.isDigit()) {
                            setFormat(i, 1, this->colorByType(SyntaxHighlighter::Number));
                            endStateOfLastPara = Digit;
                        }
                        // probably an operator
                        else if (ch.isSymbol() || ch.isPunct()) {
                            setFormat(i, 1, this->colorByType(SyntaxHighlighter::Operator));
                        }
                    }
                }
            } break;
            case Comment: {
                setFormat(i, 1, this->colorByType(SyntaxHighlighter::Comment));
            } break;
            case Literal1: {
                setFormat(i, 1, this->colorByType(SyntaxHighlighter::String));
                if (ch == QLatin1Char('"')) endStateOfLastPara = Standard;
            } break;
            case Literal2: {
                setFormat(i, 1, this->colorByType(SyntaxHighlighter::String));
                if (ch == QLatin1Char('\'')) endStateOfLastPara = Standard;
            } break;
            case Blockcomment1: {
                setFormat(i, 1, this->colorByType(SyntaxHighlighter::BlockComment));
                if (i >= 2 && ch == QLatin1Char('"') && text.at(i - 1) == QLatin1Char('"')
                    && text.at(i - 2) == QLatin1Char('"'))
                    endStateOfLastPara = Standard;
            } break;
            case Blockcomment2: {
                setFormat(i, 1, this->colorByType(SyntaxHighlighter::BlockComment));
                if (i >= 2 && ch == QLatin1Char('\'') && text.at(i - 1) == QLatin1Char('\'')
                    && text.at(i - 2) == QLatin1Char('\''))
                    endStateOfLastPara = Standard;
            } break;
            case DefineName: {
                if (ch.isLetterOrNumber() || ch == QLatin1Char(' ') || ch == QLatin1Char('_')) {
                    setFormat(i, 1, this->colorByType(SyntaxHighlighter::Defname));
                }
                else {
                    if (ch.isSymbol() || ch.isPunct()) setFormat(i, 1, this->colorByType(SyntaxHighlighter::Operator));
                    endStateOfLastPara = Standard;
                }
            } break;
            case ClassName: {
                if (ch.isLetterOrNumber() || ch == QLatin1Char(' ') || ch == QLatin1Char('_')) {
                    setFormat(i, 1, this->colorByType(SyntaxHighlighter::Classname));
                }
                else {
                    if (ch.isSymbol() || ch.isPunct()) setFormat(i, 1, this->colorByType(SyntaxHighlighter::Operator));
                    endStateOfLastPara = Standard;
                }
            } break;
            case Digit: {
                if (ch.isDigit() || ch == QLatin1Char('.')) {
                    setFormat(i, 1, this->colorByType(SyntaxHighlighter::Number));
                }
                else {
                    if (ch.isSymbol() || ch.isPunct()) setFormat(i, 1, this->colorByType(SyntaxHighlighter::Operator));
                    endStateOfLastPara = Standard;
                }
            } break;
        }

        prev = ch;
        i++;
    }

    // only block comments can have several lines
    if (endStateOfLastPara != Blockcomment1 && endStateOfLastPara != Blockcomment2) {
        endStateOfLastPara = Standard;
    }

    setCurrentBlockState(endStateOfLastPara);
}
