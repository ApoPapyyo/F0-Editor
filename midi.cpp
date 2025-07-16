#include "midi.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <cmath>
#include <QDebug>

Note::Note()
    : _name(None)
    , _oct(0)
{}

Note::Note(Note &n)
    : _name(n._name)
    , _oct(n._oct)
{}

Note::Note(eNoteName n, int o)
    : _name(n)
    , _oct(o)
{}

Note::Note(QString str)
    : _name(None)
    , _oct(0)
{
    *this = fromStr(str);
}

Note::Note(double f)
    : _name(None)
    , _oct(0)
{
    *this = fromHz(f);
}

Note::~Note()
{}

Note Note::fromStr(const QString str)
{
    QRegularExpression re("^([A-Ga-g])(\\+\\+|\\-\\-|##|bb|\\+|\\-|#|b|x)?(\\d+)$");
    QRegularExpressionMatch match = re.match(str.trimmed());

    if (!match.hasMatch()) {
        qDebug() << "Match Error";
        return Note(None, 0);
    }

    QChar name = match.captured(1)[0].toLower();
    QString s = match.captured(2);
    eNoteName n;
    switch(name.unicode()) {
    case 'a':
        n = A;
        break;
    case 'b':
        n = B;
        break;
    case 'c':
        n = C;
        break;
    case 'd':
        n = D;
        break;
    case 'e':
        n = E;
        break;
    case 'f':
        n = F;
        break;
    case 'g':
        n = G;
        break;
    default:
        n = None;
    }
    int tmp(static_cast<int>(n));
    if (QStringList({"#", "+"}).contains(s)) {
        tmp++;
        if(tmp > 12) tmp = 1;
    } else if (QStringList({"b", "-"}).contains(s)) {
        tmp--;
        if(tmp <= 0) tmp = 12;
    } else if (QStringList({"x", "##", "++"}).contains(s)) {
        tmp+=2;
        if(tmp > 12) tmp %= 12;
    } else if (QStringList({"bb", "--"}).contains(s)) {
        tmp-=2;
        if(tmp <= 0) tmp+=12;
    }
    n = static_cast<eNoteName>(tmp);
    int octave = match.captured(3).toInt();

    return Note(n, octave);
}

QString Note::toStr() const
{
    QString n;
    switch(_name) {
    case None:
        n = "None";
        break;
    case C:
        n = "C";
        break;
    case Cs:
        n = "C#";
        break;
    case D:
        n = "D";
        break;
    case Ds:
        n = "D#";
        break;
    case E:
        n = "E";
        break;
    case F:
        n = "F";
        break;
    case Fs:
        n = "F#";
        break;
    case G:
        n = "G";
        break;
    case Gs:
        n = "G#";
        break;
    case A:
        n = "A";
        break;
    case As:
        n = "A#";
        break;
    case B:
        n = "B";
        break;
    }
    QString ret(n);
    if (_name != None) ret += QString("%1").arg(_oct);
    return ret;
}

double Note::toHz(double A4) const
{
    if (_name == None) return 0.0;
    int name(static_cast<int>(_name) - 10);
    int oct(_oct - 4);
    double sound(oct*12 + name);
    return std::pow(2.0, sound/12.0)*A4;
}
// 2^(x/12) = s
// x/12 = log2(s)
// x = 12log2(s)

Note Note::fromHz(double f, double A4)
{
    if (f <= 0.0) return Note();
    double s(f/A4);
    double pitch(12.0*std::log2(s));
    int pitch_(std::round(pitch) + 12*4 + 10);
    int oct(pitch_ >= 0 ? (pitch_%12 == 0 ? pitch_/12 - 1 : pitch_/12) : 0);
    int name(pitch_ >= 0 ? (pitch_%12 == 0 ? 12 : pitch_%12) : 0);
    return Note(static_cast<eNoteName>(name), oct);
}
