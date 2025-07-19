#ifndef MIDIEDITOR_H
#define MIDIEDITOR_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QGridLayout>
#include <QScrollBar>
#include <QSplitter>
#include "pitcheditor.h"
#include "piano.h"

class MidiEditor : public QWidget
{
    Q_OBJECT
public:
    explicit MidiEditor(QWidget *parent = nullptr);
    ~MidiEditor();
    PitchEditor *get_pe() const;
protected:
    void wheelEvent(QWheelEvent *ev);
    void paintEvent(QPaintEvent *ev);
private:
    QGridLayout *_lay;
    QSplitter *_sp;
    PitchEditor *_pe;
    Piano *_piano;
    QScrollBar *_scrx, *_scry;
    int x_zoom_offset, y_zoom_offset;
    int x_zoom_max, y_zoom_max;


signals:
    void scrolled(int x, int y);
    void zoomed(int x, int y);
private slots:
    void scrollEventX(int x);
    void scrollEventY(int y);
};

#endif // MIDIEDITOR_H
