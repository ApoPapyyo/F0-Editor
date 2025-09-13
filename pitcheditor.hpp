#ifndef PITCHEDITOR_H
#define PITCHEDITOR_H
#include "note.hpp"
#include "f0.hpp"
#include <QWidget>
#include <QPainter>
#include <QBitArray>
#include <QMap>
#include <QPoint>
#include "synth.h"

class PitchEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PitchEditor(QWidget* parent = nullptr);
    enum class eMouseMode {
        Select,
        Write,
        Erase
    };
    enum class eAction {
        PitchShift,
        PitchDel,
        PitchMod
    };
    enum class eMouseAction {
        None,
        PitchDrag,
    };
    enum class eScaleMode {
        Time,
        Beat
    };

    class Area {
        QPoint* ref;
        QPoint* var;
    public:
        Area();
        Area(const QPoint& ref, const QPoint& var);
        ~Area();
        QPoint getRef() const;
        QPoint getVar() const;
        QRect getRect() const;
        int getWidth() const;
        int getHeight() const;
        void reset();
        void setRef(const QPoint& ref);
        void setVar(const QPoint& var);
        bool isSet() const;
        bool isInside(const QPoint& n) const;
        bool isEmpty() const;
    };

    class OprLog {
        struct Data {
            eAction action;
            QBitArray target;
            double diff;
            QList<Note> now;
            QList<Note> past;
        };
        int index;
        QList<Data> data;
    public:
        F0 f0_data;
        OprLog(QWidget* parent);
        QString getActionName() const;
        void pushShift(const QBitArray& target, double diff);
        void pushWrite(size_t target_idx, Note& now);
        void pushErase(size_t target_idx);
        void pushWrite(const QBitArray& target, QList<Note>& now);
        void pushErase(const QBitArray& target);
        void undo();
        void redo();
        bool ableToUndo() const;
        bool ableToRedo() const;
    };
    struct Offset {
        int x, y;
    };
    struct Scale {
        double x;
        int y;
    };
    struct Selection {
        QBitArray target;
        Area rect;
    };
    struct Mouse {
        eMouseMode mode;
        QPoint pos;
        bool lclick, mclick, rclick;
        Selection select;
    };
    struct ScaleConfig {
        eScaleMode mode;
        double BPM, beat_offset;
    };

    struct Config {
        int oct_max, oct_min;
        double A4;
        ScaleConfig scale;
    };

    bool init;

    Offset offset;
    Scale scale;
    Mouse mouse;
    OprLog log;
    Config conf;

    Offset getScrollMax() const;
    Scale getScaleMax() const;

    void setScrollOffset(int x, int y);
    void addScrollOffset(int x, int y);

    void setScale(double x, int y);
    void addScale(double x, int y);
public slots:
    void openF0(const QString& path);
    void closeF0();
    void saveF0();
    void saveF0as(const QString& path);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
public:
    void closeEvent(QCloseEvent*) override;
private:
    int notePos(const Note& n) const;
    Note posNote(int y) const;

    int curPos(int cur) const;
    int posCur(int x) const;
signals:
    void titleChange(const QString& title);
    void stbarChange(const QString& mes);

};

#endif // PITCHEDITOR_H
