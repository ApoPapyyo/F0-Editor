#include <regex>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <vector>
#include "note.hpp"

//-INFINITY: 無音(0Hz)

Note::Note()
    : _interval(-INFINITY)
{}

Note::Note(const Note &n)
    : _interval(n._interval)
{}

Note::Note(eNoteName n, int o, double c)
    : _interval(-INFINITY)
{
    c /= 100.0;
    if(n == eNoteName::None || c == -INFINITY || c == INFINITY) return;
    _interval = (o - 4) * 12 + static_cast<int>(n) - 10;
    _interval += c;
}

Note::Note(const string str)
    : _interval(-INFINITY)
{
    *this = fromStr(str);
}
//周波数から
Note::Note(const double f, const double A4)
    : _interval(-INFINITY)
{
    *this = fromHz(f, A4);
}

Note::~Note()
{}

Note Note::fromStr(const string str)
{
    regex re("^([A-Ga-g])(\\+\\+|\\-\\-|##|bb|#|b|x)?((?:0|[1-9]\\d*)|-(?:[1-9]\\d*))([+-]\\d+(?:\\.\\d+)?)?$");
    smatch match;
    if (!regex_match(str, match, re)) {
        return Note();
    }

    string n(match.str(1)), s(match.str(2)), oc(match.str(3)), pc(match.str(4));
    transform(n.begin(), n.end(), n.begin(),
        [](unsigned char c) {
            return tolower(c);
    });
    char name(n[0]);

    vector<string> sharps({"#", "♯"}), flats({"b", "♭"}), dsharps({"x", "##", "♯♯"}), dflats({"bb", "♭♭"});
    auto in = [](string &synbol, vector<string> &list) -> bool {
        if(find(list.begin(), list.end(), synbol) != list.end()) return true;
        return false;
    };

    int ss(0);
    if(in(s, sharps)) {
        ss = 1;
    } else if(in(s, dsharps)) {
        ss = 2;
    } else if(in(s, flats)) {
        ss = -1;
    } else if(in(s, dflats)) {
        ss = -2;
    }
    double p(0.0);
    if(!pc.empty()) {
        p = stod(pc);
    }
    double interval(p/100.0 + 12*(stoi(oc)-4) + ss);
    int tmp(name - 'a');
    if(tmp == 1) {//b
        tmp = 2;
    } else if(tmp == 2) {//c
        tmp = -9;
    } else if(tmp == 3) {//d
        tmp = -7;
    } else if(tmp == 4) {//e
        tmp = -5;
    } else if(tmp == 5) {//f
        tmp = -4;
    } else if(tmp == 6) {//g
        tmp = -2;
    }
    interval += tmp;
    Note ret;
    ret._interval = interval;
    return ret;
}

string Note::toStr(bool showCents, int precision) const
{
    if(_interval == -INFINITY) return "None";
    eNoteName _name = getName();
    stringstream s;
    switch(_name) {
    case eNoteName::C:
        s << "C";
        break;
    case eNoteName::Cs:
        s << "C#";
        break;
    case eNoteName::D:
        s << "D";
        break;
    case eNoteName::Ds:
        s << "D#";
        break;
    case eNoteName::E:
        s << "E";
        break;
    case eNoteName::F:
        s << "F";
        break;
    case eNoteName::Fs:
        s << "F#";
        break;
    case eNoteName::G:
        s << "G";
        break;
    case eNoteName::Gs:
        s << "G#";
        break;
    case eNoteName::A:
        s << "A";
        break;
    case eNoteName::As:
        s << "A#";
        break;
    case eNoteName::B:
        s << "B";
        break;
    }
    s << getOct();
    double c;
    if((c = getCent()) != 0.0 && showCents) {
        if(c >= 0.0) s << "+";
        s << fixed << setprecision(precision);
        s << c;
    }
    return s.str();
}

double Note::toHz(const double A4) const
{
    if (_interval == -INFINITY) return 0.0;
    return std::pow(2.0, _interval/12.0)*A4;
}
// 2^(x/12) = s
// x/12 = log2(s)
// x = 12log2(s)

Note Note::fromHz(const double f, const double A4)
{
    if (f <= 0.0 || A4 <= 0.0) return Note();
    double s(f/A4);
    Note ret;
    ret._interval = 12.0*std::log2(s);
    return ret;
}

Note::eNoteName Note::getName() const
{
    if(_interval == -INFINITY) return eNoteName::None;
    double i = _interval;
    while(i < 0) i+=12.0;
    i = round(i) + 9;
    return static_cast<eNoteName>(static_cast<int>(i)%12 + 1);
}

double Note::getCent() const
{
    if(_interval == -INFINITY) return -INFINITY;
    double i = _interval;
    while(i < 0) i+=12.0;
    double r = round(i);
    return (i - r)*100;
}

int Note::getOct() const
{
    if(_interval == -INFINITY) return 0;
    double o = (_interval + 58 - static_cast<int>(getName()) - getCent()/100.0)/12.0;
    return static_cast<int>(round(o));
}

double operator-(const Note &_a, const Note &_b)
{
    if(_a._interval == -INFINITY || _b._interval == -INFINITY) return 0.0;
    return _a._interval - _b._interval;
}

Note operator+(const Note &a, const double b)
{
    if(a._interval == -INFINITY) return Note();
    Note ret;
    ret._interval = a._interval + b;
    return ret;
}

Note operator-(const Note &a, const double b)
{
    if(a._interval == -INFINITY) return Note();
    Note ret;
    ret._interval = a._interval - b;
    return ret;
}

Note &Note::operator+=(const double b)
{
    if(_interval == -INFINITY) return *this;
    _interval += b;
    return *this;
}

Note &Note::operator-=(const double b)
{
    if(_interval == -INFINITY) return *this;
    _interval -= b;
    return *this;
}

bool operator==(const Note &a, const Note &b)
{
    if (a._interval == b._interval) return true;
    return false;
}

bool operator!=(const Note &a, const Note &b)
{
    if (a._interval == b._interval) return false;
    return true;
}

bool operator<(const Note &a, const Note &b)
{
    if(a._interval < b._interval) return true;
    else return false;
}

bool operator>(const Note &a, const Note &b)
{
    if(a._interval > b._interval) return true;
    else return false;
}

bool operator<=(const Note &a, const Note &b)
{
    if(a._interval <= b._interval) return true;
    else return false;
}

bool operator>=(const Note &a, const Note &b)
{
    if(a._interval >= b._interval) return true;
    else return false;
}

bool Note::nearEqual(const Note &a, const Note &b)
{
    if(a.getOct() != b.getOct()) return false;
    if(a.getName() != b.getName()) return false;
    return true;
}
