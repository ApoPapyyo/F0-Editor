#include "pitcheditor.h"
#include <cmath>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>

const int PitchEditor::oct_max = 8;
PitchEditor::PitchEditor(QWidget *parent)
    : QWidget{parent}
    , piano_keyboard_width(20)
    , note_size(1)
    , now(Note(Note::C, 0.0, 4))
    , centre(Note(Note::C, 0.0, 4))
    , x_scroll_offset(0)
    , y_scroll_offset((Note(Note::B, 0.0, 8)-centre)*piano_keyboard_width)
    , init(false)
    , f0()
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    this->grabGesture(Qt::PinchGesture);
}

void PitchEditor::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.setBrush(QColor(200, 200, 200));
    painter.drawRect(QRect(0, 0, width(), height()));
    if(!init) {
        y_scroll_offset -= height()/2;
        init = true;
    }
    drawF0(painter);
}

void PitchEditor::drawF0(QPainter &painter)
{
    int i, j(x_scroll_offset/note_size);
    painter.setBrush(QColor(255, 0, 0));
    for(i = 0; i < width() && j < f0.getDataSize(); i+=note_size) {
        Note d(f0.getData(j));
        if(d != Note()) painter.drawRect(QRect(i, soundPosition(d), note_size, 1));
        j++;
    }
}
/*
void PitchEditor::wheelEvent(QWheelEvent *ev)
{
    Qt::KeyboardModifiers mods = ev->modifiers();
    auto scrx = ev->angleDelta().x(), scry = ev->angleDelta().y();
    if (mods & Qt::ShiftModifier) {
        auto tmp = scrx;
        scrx = scry;
        scry = tmp;
    }
    if (mods & Qt::ControlModifier) {
        note_size -= scrx;
        if (note_size < 0 || note_size > 300) note_size += scrx;

        piano_keyboard_width -= scry/10;
        if (piano_keyboard_width < 10 || piano_keyboard_width > 300) piano_keyboard_width += scry/10;
        else {
            for (y_scroll_offset = 0; centre != mouseSound(QPoint(0, height()/2)); y_scroll_offset++);
        }
    } else {
        x_scroll_offset -= scrx;
        if(x_scroll_offset < 0) x_scroll_offset = 0;
        else if((x_scroll_offset+width()) > 10000) x_scroll_offset += scrx;

        y_scroll_offset -= scry;
        if(y_scroll_offset < 0) y_scroll_offset = 0;
        else if ((y_scroll_offset+height())/piano_keyboard_width/12 > oct_max) y_scroll_offset += scry;
        centre = mouseSound(QPoint(0, height()/2));
    }
    ev->accept();
    update();
    emit scrolled(scrx, scry);
}
*/


Note PitchEditor::mouseSound(QPoint p) const
{
    int y(y_scroll_offset+p.y()-piano_keyboard_width/2);
    double y_ = static_cast<double>(y)/piano_keyboard_width;
    y_ = 12*oct_max - y_;
    return Note(Note::B, y_, 0);
}

void PitchEditor::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    now = mouseSound(pos);
    emit mouseMoved(tr("%1Hzは%2(%3セント). A4=440Hzとした%2は%4Hz. C0との音程は%5.").arg(now.toHz()).arg(now.toStr()).arg(now.getCent()*100).arg(Note(now.getName(), 0.0, now.getOct()).toHz()).arg(now-Note(Note::C, 0.0, 0)));
}

int PitchEditor::soundPosition(Note n)
{
    double iv_(Note(Note::B, 0.0, oct_max) - n);
    iv_ *= piano_keyboard_width;
    int iv(std::round(iv_)-y_scroll_offset+piano_keyboard_width/2);
    return iv;
}

int PitchEditor::get_x_scroll_max() const
{
    return f0.getDataSize()*note_size - width();
}

int PitchEditor::get_y_scroll_max() const
{
    return oct_max*12*piano_keyboard_width - height();
}

int PitchEditor::get_x_scroll_offset() const
{
    return x_scroll_offset;
}

int PitchEditor::get_y_scroll_offset() const
{
    return y_scroll_offset;
}

void PitchEditor::set_x_scroll_offset(int x)
{
    if(0 <= x && x+width() < f0.getDataSize()*note_size) {
        x_scroll_offset = x;
        update();
    }
}

void PitchEditor::set_y_scroll_offset(int y)
{
    if(0 <= y && y+height() < oct_max*12*piano_keyboard_width) {
        y_scroll_offset = y;
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        centre = mouseSound(QPoint(0, height()/2));
        update();
    }
}

int PitchEditor::get_x_zoom() const
{
    return note_size;
}

int PitchEditor::get_y_zoom() const
{
    return piano_keyboard_width;
}

void PitchEditor::set_x_zoom(int x)
{
    if(1 <= x && x <= 100) {
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        note_size = x;
        update();
    }
}

void PitchEditor::set_y_zoom(int y)
{
    if(5 <= y && y <= 100) {
        piano_keyboard_width = y;
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        for (y_scroll_offset = 0; centre < mouseSound(QPoint(0, height()/2)); y_scroll_offset++);//ズームの中心を固定するため
        update();
    }
}

void PitchEditor::open_f0()
{
    QFileInfo fn(QFileDialog::getOpenFileName(
        this,
        "ファイルを開く",
        "",
        "F0ファイル (*.csv *.f0);;"
        ));
    if (f0.openF0(fn)) {
        update();
    }

}

void PitchEditor::close_f0()
{
    f0.closeF0();
    x_scroll_offset = 0;
    update();
}
