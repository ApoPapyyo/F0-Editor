#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "midi.h"
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>

class Piano;

class PitchEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PitchEditor(QWidget *parent = nullptr);
    friend class Piano;

protected:
    void paintEvent(QPaintEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    void draw_piano(QPainter &painter);
    Note mouseSound() const;
    int soundPosition(Note n);
    int piano_keyboard_width;
    int x_scroll_offset;
    int y_scroll_offset;
    static const int oct_max;
signals:
    void mouseMoved(const QString &info);
    void scrolled(int x, int y);
    void zoomed(int x, int y);
    void sync_offset(int x, int y);

};

#endif // PITCHEDITOR_H
