#ifndef PIANO_H
#define PIANO_H

#include <QWidget>
#include "pitcheditor.h"

class Piano : public QWidget
{
    Q_OBJECT
public:
    explicit Piano(PitchEditor *pe, QWidget *parent = nullptr);
    friend class PitchEditor;
protected:
    void paintEvent(QPaintEvent *ev);
private:
    PitchEditor *pe;
    int &keysize;
public slots:
    void scrolled(int x, int y);

signals:
};

#endif // PIANO_H
