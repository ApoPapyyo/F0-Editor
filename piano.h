#ifndef PIANO_H
#define PIANO_H
#include "midi.h"
#include <QWidget>
#include "pitcheditor.h"
#include "synth.h"
class Piano : public QWidget
{
    Q_OBJECT
public:
    explicit Piano(PitchEditor *pe, QWidget *parent = nullptr);
    friend class PitchEditor;
protected:
    void paintEvent(QPaintEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
private:
    PitchEditor *pe;
    int &keysize;
    int &y_scroll_offset;
    Synth synth;
    Note pushing;

public slots:
    void scrolled(int x, int y);

signals:
};

#endif // PIANO_H
