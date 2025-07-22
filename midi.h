#ifndef Note_H
#define Note_H
#include <QString>

class Note {
public:
    enum eNoteName {
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
    eNoteName _name;
    double _cent;
    int _oct;
    static Note normalize(const Note &a);
public:
    Note();
    Note(const Note &);
    Note(QString);
    Note(eNoteName, double, int);
    Note(const double f, const double A4 = 440.0);
    ~Note();
    eNoteName getName() const;
    double getCent() const;
    int getOct() const;
    QString toStr() const;
    double toHz(const double A4 = 440.0) const;
    static Note fromStr(QString);
    static Note fromHz(const double f, const double A4 = 440.0);
    friend Note operator+(const Note &a, const int b);
    friend Note operator-(const Note &a, const int b);
    friend Note operator+(const Note &a, const double b);
    friend Note operator-(const Note &a, const double b);
    friend bool nearEqual(const Note &a, const Note &b);//微分音は無視
    friend bool operator==(const Note &a, const Note &b);//微分音含め完璧に同じか
    friend bool operator!=(const Note &a, const Note &b);
    friend bool operator<(const Note &a, const Note &b);//aがbよりも低い音
    friend bool operator>(const Note &a, const Note &b);//aがbよりも高い音
    friend bool operator<=(const Note &a, const Note &b);
    friend bool operator>=(const Note &a, const Note &b);
    Note &operator+=(const int a);
    Note &operator-=(const int a);
    friend double operator-(const Note &a, const Note &b);//音程を半音の数で
};



#endif // Note_H
