#ifndef Note_H
#define Note_H
#include <string>
using namespace std;

class Note {
public:
    enum class eNoteName {
        None = 0,
        C,
        Cs,
        D,
        Ds,
        E,
        F,
        Fs,
        G,
        Gs,
        A,
        As,
        B
    };
private:
    double _interval;//A4から数えて
public:
    Note();
    Note(const Note &org);
    Note(const string str);//文字列から
    Note(eNoteName n, int o, double c);//音名、オクターブ、セント
    Note(const double f, const double A4 = 440.0);
    ~Note();
    eNoteName getName() const;
    double getCent() const;
    int getOct() const;
    string toStr(bool showCents = false, int precision = 2) const;
    double toHz(const double A4 = 440.0) const;
    static Note fromStr(const string str);
    static Note fromHz(const double f, const double A4 = 440.0);
    Note &operator+=(const double a);
    Note &operator-=(const double a);
    friend Note operator+(const Note &a, const double b);
    friend Note operator-(const Note &a, const double b);
    static bool nearEqual(const Note &a, const Note &b);//微分音は無視
    friend bool operator==(const Note &a, const Note &b);//微分音含め完璧に同じか
    friend bool operator!=(const Note &a, const Note &b);
    friend bool operator<(const Note &a, const Note &b);//aがbよりも低い音
    friend bool operator>(const Note &a, const Note &b);//aがbよりも高い音
    friend bool operator<=(const Note &a, const Note &b);
    friend bool operator>=(const Note &a, const Note &b);
    friend double operator-(const Note &a, const Note &b);//音程を半音の数で
};

#endif // Note_H
