#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "midi.h"
#include "f0.h"
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QGestureEvent>
#include <QMap>


class Piano;

class PitchEditor : public QWidget
{
    Q_OBJECT
public:
    enum class eMouseMode {
        Select,
        Write,
        Erase
    };
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
    void setMode(eMouseMode mode);
    eMouseMode getMode() const;

protected:
    void paintEvent(QPaintEvent *ev);
    //void wheelEvent(QWheelEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void keyPressEvent(QKeyEvent *ev);


private:
    enum class eAction {
        PitchShift,
        PitchDel,
        PitchMod
    };
    enum class eMouseAction {
        None,
        PitchDrag,
    };

    class Area {
        int ref, var;
    public:
        Area();
        Area(int ref, int var);
        int getStart() const;
        int getEnd() const;
        int getRef() const;
        int getVar() const;
        void reset();
        void setRef(int n);
        void setVar(int n);
        bool seted() const;
        QList<int> getIndex() const;
    };

    class Area2 {
        QPoint *ref;
        QPoint *var;
    public:
        Area2();
        Area2(const QPoint &ref, const QPoint &var);
        QPoint getRef() const;
        QPoint getVar() const;
        QRect getRect() const;
        int getWidth() const;
        int getHeight() const;
        void reset();
        void setRef(const QPoint &ref);
        void setVar(const QPoint &var);
        bool seted() const;
        bool isin(const QPoint &n) const;
        bool isempty() const;
    };

    struct ModLog {
        eAction action;
        Area area;
        union data {
            double diff;
            QList<double> data;
        };
    };
    Note mouseSound(QPoint p) const;
    Note mouseSound(int y) const;
    Note pos2sound(QPoint p) const;
    Note pos2sound(int y) const;
    int soundPosition(Note n);
    void drawF0(QPainter &painter);
    void drawSelect(QPainter &painter);
    int question() const;
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
    Area2 rectselect;
    QList<int> selected;
    QList<ModLog> modlog;
    eMouseAction mouse;
    int draggedid;
    QPoint mousexy;
    int sel;
    bool lclick;
    static const int oct_max;
signals:
    void mouseMoved(const QString &info);
    void scrolleds(int x, int y);
    void titlechange(const QString &title);
public slots:
    void open_f0();
    void close_f0();
    void save_f0();
    void save_f0_as();
    void clicked_other();
    void closeEvent(QCloseEvent *ev);

};

#endif // PITCHEDITOR_H
