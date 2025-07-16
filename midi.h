#ifndef Note_H
#define Note_H
#include <QString>

class Note {
public:
    enum eNoteName {
        None,
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
    eNoteName name;
    int oct;

    static Note fromStr(QString);
public:
    Note();
    Note(Note &);
    Note(QString);
    Note(eNoteName, int);
    ~Note();
    QString toStr() const;

};

#endif // Note_H
