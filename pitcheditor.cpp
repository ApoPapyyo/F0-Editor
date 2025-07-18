#include "pitcheditor.h"
#include <cmath>
#include <QCursor>

const int PitchEditor::oct_max = 8;
PitchEditor::PitchEditor(QWidget *parent)
    : QWidget{parent}
    , piano_keyboard_width(20)
    , note_size(50)
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
}

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

void PitchEditor::set_x_scroll_offset(int x)
{
    if(0 <= x || x+width() <= f0.getDataSize()) x_scroll_offset = x;
}

void PitchEditor::set_y_scroll_offset(int y)
{
    if(0 <= y || (y+height())/piano_keyboard_width/12 <= oct_max) y_scroll_offset = y;
}

