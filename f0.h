#ifndef F__H
#define F__H

#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QList>
#include <stdint.h>
#include "midi.h"

class F0
{
public:
    F0();
    F0(QFileInfo &path);
    ~F0();
    int openF0(QFileInfo &path);
    void closeF0();
    int getDataSize() const;
    Note getData(int i) const;
private:
    QFileInfo _path;
    QList<Note> _data;
};

#endif // F__H
