#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "midi.h"
#include "f0.h"
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QGestureEvent>


class Piano;

class PitchEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PitchEditor(QWidget *parent = nullptr);
    friend class Piano;
    void set_x_scroll_offset(int);
    void set_y_scroll_offset(int);
    void set_piano_keyboard_width(int);
    void set_note_size(int);

protected:
    void paintEvent(QPaintEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    void draw_piano(QPainter &painter);
    Note mouseSound(QPoint p) const;
    int soundPosition(Note n);
    int piano_keyboard_width;
    int note_size;
    Note now, centre;
    int x_scroll_offset;
    int y_scroll_offset;
    bool init;
    F0 f0;
    static const int oct_max;
signals:
    void mouseMoved(const QString &info);
    void scrolled(int x, int y);
    void zoomed(int x, int y);
    void sync_offset(int x, int y);

};

#endif // PITCHEDITOR_H
