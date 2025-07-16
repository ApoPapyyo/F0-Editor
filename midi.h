#ifndef Note_H
#define Note_H
#include <QString>
/*
class Interval {
    enum eType {
        DA,//重増
        A,//増
        Mj,//長
        P,//完全
        Mn,//短
        D,//減
        DD//重減
    };
};
*/
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
    int _oct;
public:
    Note();
    Note(Note &);
    Note(QString);
    Note(eNoteName, int);
    Note(double f);
    ~Note();
    QString toStr() const;
    double toHz(double A4 = 440.0) const;
    static Note fromStr(QString);
    static Note fromHz(double f, double A4 = 440.0);

};

#endif // Note_H
