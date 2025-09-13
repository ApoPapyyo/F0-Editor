#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "note.h"
#include "f0.h"
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QGestureEvent>
#include <QBitArray>
#include <QMap>
#include "synth.h"


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

    class ModLog {
        struct Data {
            eAction action;
            QBitArray target;
            double diff;
            QList<Note> now;
            QList<Note> pasts;
        };
        int index;
        QList<Data> data;
    public:
        ModLog();
        ~ModLog();
        QString getActionName() const;
        void pushShiftLog(QBitArray &target, double diff);
        void pushWriteLog(QBitArray &target, QList<Note> &now, QList<Note> &pasts);
        void pushEraseLog(QBitArray &target, QList<Note> &pasts);
        void undo(F0 &f0);
        void redo(F0 &f0);
        bool undo_able() const;
        bool redo_able() const;
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
    QBitArray select;
    ModLog modlog;
    eMouseAction mouse;
    int draggedid;
    QPoint mousexy;
    int sel;
    bool lclick;
    double dragdiff;
    QMap<int,Note> writed, writepast;
    Synth synth;
    QList<double> cache;
    bool changed;
    int pcursor;
    bool follow_pcursor;
    static const int oct_max;
signals:
    void mouseMoved(const QString &info);
    void scrolleds(int x, int y);
    void titlechange(const QString &title);
    void undo_redo_tgl(bool undo, bool redo);
public slots:
    void open_f0();
    void close_f0();
    void save_f0();
    void save_f0_as();
    void clicked_other();
    void closeEvent(QCloseEvent *ev);
    void redo();
    void undo();
    void play();
    void stop();
    void temp_stop();
    void play_cursor_update(int frame);

};

#endif // PITCHEDITOR_H
