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
    int get_x_scroll_offset() const;
    int get_y_scroll_offset() const;
    int get_x_scroll_max() const;
    int get_y_scroll_max() const;
    int get_x_zoom() const;
    int get_y_zoom() const;
    void set_x_scroll_offset(int x);
    void set_y_scroll_offset(int y);
    void set_x_zoom(int x);
    void set_y_zoom(int y);

protected:
    void paintEvent(QPaintEvent *ev);
    //void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    void draw_piano(QPainter &painter);
    Note mouseSound(QPoint p) const;
    int soundPosition(Note n);
    void drawF0(QPainter &painter);
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
    void scrolleds(int x, int y);
public slots:
    void open_f0();
    void close_f0();

};

#endif // PITCHEDITOR_H
