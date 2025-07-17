#include "midi.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <cmath>
#include <QDebug>

Note::Note()
    : _name(None)
    , _cent(0.0)
    , _oct(0)
{}

Note::Note(const Note &n)
    : _name(n._name)
    , _cent(n._cent)
    , _oct(n._oct)
{}

Note::Note(eNoteName n, double c, int o)
    : _name(n)
    , _cent(c)
    , _oct(o)
{
    if(-1.0 >= _cent || _cent >= 1.0) {
        int t(static_cast<int>(_cent));
        _cent -= t;
        *this = *this + t;
    }
    if (-0.5 >= _cent) {
        _cent = 1.0 + _cent;
        *this = *this - 1;
    } else if (_cent > 0.5) {
        _cent = _cent - 1.0;
        *this = *this + 1;
    }
}

Note::Note(QString str)
    : _name(None)
    , _cent(0.0)
    , _oct(0)
{
    *this = fromStr(str);
}

Note::Note(const double f, const double A4)
    : _name(None)
    , _cent(0.0)
    , _oct(0)
{
    *this = fromHz(f, A4);
}

Note::~Note()
{}

Note Note::fromStr(const QString str)
{
    QRegularExpression re("^([A-Ga-g])(\\+\\+|\\-\\-|##|bb|\\+|\\-|#|b|x)?(\\d+)$");
    QRegularExpressionMatch match = re.match(str.trimmed());

    if (!match.hasMatch()) {
        qDebug() << "Match Error";
        return Note(None, 0.0, 0);
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

    return Note(n, 0.0, octave);
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
    //if (_cent != 0.0) ret += QString("(%1 cent)").arg(_cent);
    return ret;
}

double Note::toHz(const double A4) const
{
    if (_name == None) return 0.0;
    int name(static_cast<int>(_name) - 10);
    int oct(_oct - 4);
    double sound(oct*12.0 + (_cent+name));
    return std::pow(2.0, sound/12.0)*A4;
}
// 2^(x/12) = s
// x/12 = log2(s)
// x = 12log2(s)

Note Note::fromHz(const double f, const double A4)
{
    if (f <= 0.0) return Note();
    double s(f/A4);
    double pitch(12.0*std::log2(s));
    pitch += 12*4 + 10;
    int pitch_(std::round(pitch));
    int oct(pitch_ >= 0 ? (pitch_%12 == 0 ? pitch_/12 - 1 : pitch_/12) : 0);
    int name(pitch_ >= 0 ? (pitch_%12 == 0 ? 12 : pitch_%12) : 0);
    return Note(static_cast<eNoteName>(name), pitch - pitch_, oct);
}

Note operator+(const Note &a, int b)
{
    if(b == 0 || a._name == Note::None) return a;
    else if(b > 0) {
        int t(static_cast<int>(a._name)), o(a._oct);
        t += b;
        o += (t-1)/12;
        t = (t-1)%12 + 1;
        Note::eNoteName t_(static_cast<Note::eNoteName>(t));
        return Note(t_, a._cent, o);
    } else if (b < 0) {
        return a - (-b);
    }
}

Note operator-(const Note &a, int b)
{
    if(b == 0 || a._name == Note::None) return a;
    else if(b > 0) {
        int t(static_cast<int>(a._name)), o(a._oct);
        t -= b;
        o -= (t-1)/12;
        t = (t-1)%12 + 1;
        Note::eNoteName t_(static_cast<Note::eNoteName>(t));
        return Note(t_, a._cent, o);
    } else if (b < 0) {
        return a + (-b);
    }
}

Note::eNoteName Note::getName() const
{
    return _name;
}

double Note::getCent() const
{
    return _cent;
}

int Note::getOct() const
{
    return _oct;
}

double operator-(const Note &_a, const Note &_b)
{
    const Note *a(&_a), *b(&_b);
    int sign(1);
    auto aeb_oct = [&]()->bool {
        return a->_oct == b->_oct;
    };
    auto aeb_name = [&]()->bool {
        return a->_name == b->_name;
    };
    auto aeb_cent = [&]()->bool {
        return a->_cent == b->_cent;
    };
    auto alstb_oct = [&]()->bool {
        return a->_oct < b->_oct;
    };
    auto alstb_name = [&]()->bool {
        return static_cast<int>(a->_name) < static_cast<int>(b->_name);
    };
    auto alstb_cent = [&]()->bool {
        return a->_cent < b->_cent;
    };
    auto or_none = [&]()->bool {
        return a->_name == Note::None || b->_name == Note::None;
    };
    if ((aeb_oct() && aeb_name() && aeb_cent()) || or_none()) {
        return 0.0;
    }
    else if (alstb_oct() || (aeb_oct() && alstb_name()) || (aeb_oct() && aeb_name() && alstb_cent())){
        a = &_b;
        b = &_a;
        sign *= -1;
    }

    int iv_oct(0), iv_name(0);
    double iv_cent(0.0);

    if (!aeb_oct()) {
        iv_oct = a->_oct - b->_oct;
    }
    if (!aeb_name()) {
        iv_name = static_cast<int>(a->_name) - static_cast<int>(b->_name);
    }
    if (!aeb_cent()) {
        iv_cent = a->_cent - b->_cent;
    }

    return (iv_cent + iv_name + iv_oct*12)*static_cast<double>(sign);
}
