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
    double get_x_zoom() const;
    int get_y_zoom() const;
    void set_x_scroll_offset(int x);
    void set_y_scroll_offset(int y);
    void set_x_zoom(double x);
    void set_y_zoom(int y);

protected:
    void paintEvent(QPaintEvent *ev);
    //void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void keyPressEvent(QKeyEvent *ev);


private:
    enum class eMouseMode {
        Select,
        Write,
        Erase
    };
    class area_t {
        int ref, var;
    public:
        area_t();
        int getStart() const;
        int getEnd() const;
        int getRef() const;
        int getVar() const;
        void reset();
        void setRef(int n);
        void setVar(int n);
        bool seted() const;
    };

    Note mouseSound(QPoint p) const;
    int soundPosition(Note n);
    void drawF0(QPainter &painter);
    void drawSelect(QPainter &painter);
    int piano_keyboard_width;
    double note_size;
    Note now;
    int centrex;
    Note centrey;
    int x_scroll_offset;
    int y_scroll_offset;
    bool init;
    F0 f0;
    eMouseMode mode;
    area_t selected;
    bool lclick;
    static const int oct_max;
signals:
    void mouseMoved(const QString &info);
    void scrolleds(int x, int y);
public slots:
    void open_f0();
    void close_f0();
    void clicked_other();

};

#endif // PITCHEDITOR_H
