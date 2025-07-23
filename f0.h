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
    int getFPS() const;
    int getDataSize() const;
    Note getData(int i) const;
    QList<Note> getData(int first, int size) const;
    void setData(int i, Note d);
    void setData(int first, const QList<Note> &ds);
private:
    QFileInfo _path;
    QList<Note> _data;
    int _fps;
};

#endif // F__H
