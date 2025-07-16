#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "midi.h"
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>

class PitchEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PitchEditor(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    void draw_piano(QPainter &painter);
    Note mouseSound() const;
    static int soundPosition(Note n);
    int piano_width;
    int piano_keyboard_width;
    int x_scroll_offset;
    int y_scroll_offset;
signals:
    void mouseMoved(const QString &info);


signals:
};

#endif // PITCHEDITOR_H
