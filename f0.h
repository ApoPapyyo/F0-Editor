#ifndef F__H
#define F__H

#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QList>
#include "midi.h"

class F0
{
public:
    F0();
    F0(QFileInfo &path);
private:
    QFileInfo _path;
    QList<Note> _data;
    int openF0(QFileInfo &path);
};

#endif // F__H
