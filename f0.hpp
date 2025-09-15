#ifndef F__H
#define F__H

#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QVector>
#include <cstdint>
#include <QObject>
#include <QWidget>
#include "note.hpp"

class F0
{
public:
    F0(QWidget* parent);
    int openF0(QFileInfo &path);
    void closeF0();
    int saveF0();
    int saveF0as(QFileInfo &path);
    int getFPS() const;
    bool isChanged() const;
    int getDataSize() const;
    Note getData(int i) const;
    QString getFileName() const;
    QVector<Note> getData(int first, int size) const;
    void setData(int i, Note d);
    void setData(int first, const QVector<Note> &ds);
    void setData(const QVector<int> index, Note data);
    QVector<double> getFreq(const double A4 = 440.0) const;
private:
    QFileInfo _path;
    QVector<Note> _data;
    QVector<double> _cache;
    int _fps;
    bool _changed;
    QWidget* _parent;
};

#endif // F__H
