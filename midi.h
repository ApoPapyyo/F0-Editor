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
    friend Note operator+(const Note &a, int b);
    friend Note operator-(const Note &a, int b);
    friend double operator-(const Note &a, const Note &b);//音程を半音の数で
};



#endif // Note_H
