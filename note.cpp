#include <regex>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <vector>
#include "note.h"

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
    _interval = static_cast<int>(n) - 1;
    _interval += (o + 1)*12;
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
    double interval(12*(stoi(oc)+1) + ss + p/100.0);
    int tmp(name - 'c');
    if(tmp == -2) {
        tmp = 9;
    } else if(tmp == -1) {
        tmp = 11;
    } else if(tmp == 1) {
        tmp = 2;
    } else if(tmp == 2) {
        tmp = 4;
    } else if(tmp == 3) {
        tmp = 5;
    } else if(tmp == 4) {
        tmp = 7;
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
    int oct(static_cast<int>(_interval)/12 - 5);
    int name(static_cast<int>(_interval)%12 - 9);
    double _cent(_interval - static_cast<int>(_interval));
    double sound(oct*12 + _cent+name);
    return std::pow(2.0, sound/12.0)*A4;
}
// 2^(x/12) = s
// x/12 = log2(s)
// x = 12log2(s)

Note Note::fromHz(const double f, const double A4)
{
    if (f <= 0.0 || A4 <= 0.0) return Note();
    double s(f/A4);
    double pitch(12.0*std::log2(s));
    pitch += 69.0;
    Note ret;
    ret._interval = pitch;
    return ret;
}

/*
interval: 4.3
interpoint: 0.3
ref: 4.0
range: 3.5 < interval <= 4.5
cent: 0.3

interval: 3.7
interpoint: 0.7
ref: 4.0
range: 3.5 < interval <= 4.5
cent: -0.3

interval: -0.2
interval+: 1.0 + interval = 0.8
_cent: 0.8 → -0.2
ref: 0.0
range: -0.5 < interval <= 0.5
intervalpoint: -0.2
cent: -0.2

interval: -0.5
interval+: 1.0 + interval = 0.5
_cent: 0.5
ref: -1.0
range: -1.5 < interval <= -0.5
intervalpoint: -0.5
cent: 0.5

interval: -0.6
interval+: 1.0 + interval = 0.4
_cent: 0.4
ref: -1.0
cent: 0.4

interval: -1.2
interval+: 1.0 + interval = -0.2
_cent: -0.2
ref: -1.0
cent: -0.2

interval: -1.5
interval+: 1.0 + interval = -0.5
_cent: -0.5 → 0.5
ref: -2.0
cent: 0.5

interval: -10.3
interval+: 1.0 + interval = -9.3
_cent: -0.3
ref: -10
cent: -0.3

interval: -10.5
interval+: 1.0 + interval = -9.5
_cent: -0.5 → 0.5
ref: -11
cent: 0.5
*/

double Note::get_cent(double interval)
{
    double iv(interval);
    if(iv < 0.0) {
        iv += 1.0;
    }
    double _cent(iv - static_cast<int>(iv));
    return _cent;
}

int Note::get_ref(double interval)
{
    double _cent(get_cent(interval));
    if(_cent > 0.5 && interval >= 0.0) {
        return static_cast<int>(interval) + 1;
    } else if(_cent <= -0.5 && interval < 0.0){
        return static_cast<int>(interval) - 1;
    }
    return static_cast<int>(interval);
}

Note::eNoteName Note::getName() const
{
    if (_interval == -INFINITY) return eNoteName::None;
    int r(get_ref(_interval));
    while(r < 0.0) r += 12;
    int tmp(r%12 + 1);
    return static_cast<eNoteName>(tmp);
}

double Note::getCent() const
{
    if (_interval == -INFINITY) return 0.0;
    double _cent(get_cent(_interval));
    if(_cent > 0.5) {
        _cent -= 1.0;
    } else if(_cent <= -0.5) {
        _cent += 1.0;
    }
    return _cent*100;
}

int Note::getOct() const
{
    if (_interval == -INFINITY) return 0;
    int r(get_ref(_interval));
    return r >= 0.0 ? r/12 - 1 : (r + 1)/12 - 2;
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
