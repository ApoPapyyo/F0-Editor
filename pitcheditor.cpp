#include "pitcheditor.h"
#include <cmath>
#include <QCursor>

const int PitchEditor::oct_max = 8;
PitchEditor::PitchEditor(QWidget *parent)
    : QWidget{parent}
    , piano_keyboard_width(20)
    , x_scroll_offset(0)
    , y_scroll_offset(piano_keyboard_width*(12*4)-height()/2)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void PitchEditor::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.setBrush(QColor(200, 200, 200));
    painter.drawRect(QRect(0, 0, width(), height()));
    int y(soundPosition(Note(Note::C, 0.0, 4)));
    if(0 < y && y < height()) painter.drawLine(100 - x_scroll_offset, y, 200 - x_scroll_offset, y);// (QPoint(width()/2, soundPosition(Note(Note::C, 0.0, 4))));
}

void PitchEditor::wheelEvent(QWheelEvent *ev)
{
    Qt::KeyboardModifiers mods = ev->modifiers();
    auto scry = ev->angleDelta().y();
    if (mods & Qt::ShiftModifier) {
        x_scroll_offset += scry;
        if(x_scroll_offset < 0) x_scroll_offset = 0;
        else if((x_scroll_offset+width()) > 10000) x_scroll_offset -= scry;
    } else {
        y_scroll_offset -= scry;
        if(y_scroll_offset < 0) y_scroll_offset = 0;
        else if ((y_scroll_offset+height())/piano_keyboard_width/12 > oct_max) y_scroll_offset += scry;
    }
    ev->accept();
    update();
    emit scrolled(ev->angleDelta().x(), scry);
}

Note PitchEditor::mouseSound() const
{
    QPoint p(QCursor::pos());
    p = this->mapFromGlobal(p);
    int y(y_scroll_offset+p.y()-piano_keyboard_width/2);
    double y_ = static_cast<double>(y)/piano_keyboard_width;
    y_ = 12*oct_max - y_;
    return Note(Note::B, y_, 0);
}

void PitchEditor::mouseMoveEvent(QMouseEvent *ev)
{
    Note n(mouseSound());
    emit mouseMoved(tr("%1Hzは%2(%3セント). A4=440Hzとした%2は%4Hz. C0との音程は%5.").arg(n.toHz()).arg(n.toStr()).arg(n.getCent()*100).arg(Note(n.getName(), 0.0, n.getOct()).toHz()).arg(n-Note(Note::C, 0.0, 0)));
}

int PitchEditor::soundPosition(Note n)
{
    double iv_(Note(Note::B, 0.0, oct_max) - n);
    iv_ *= piano_keyboard_width;
    int iv(std::round(iv_)-y_scroll_offset+piano_keyboard_width/2);
    return iv;
}
