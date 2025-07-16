#include "pitcheditor.h"
#include <algorithm>

const int piano_structure[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const int oct_max = 8;
PitchEditor::PitchEditor(QWidget *parent)
    : QWidget{parent}
    , piano_width(50)
    , piano_keyboard_width(20)
    , x_scroll_offset(0)
    , y_scroll_offset(piano_keyboard_width*12*3)
{
    setFocusPolicy(Qt::StrongFocus);
}

void PitchEditor::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.setBrush(QColor(200, 200, 200));
    painter.drawRect(QRect(0, 0, width(), height()));
    draw_piano(painter);
}

void PitchEditor::draw_piano(QPainter &painter)
{
    int wr(0), oct(oct_max - y_scroll_offset/piano_keyboard_width/12);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRect(QRect(0, 0, piano_width, height()));
    painter.setBrush(QColor(0, 0, 0));
    painter.drawLine(piano_width, 0, piano_width, height());
    for(int i(y_scroll_offset/piano_keyboard_width); i*piano_keyboard_width - y_scroll_offset < height(); i++) {
        if (piano_structure[11-i%12]) {
            painter.drawRect(QRect(0, i*piano_keyboard_width - y_scroll_offset, piano_width/2, piano_keyboard_width));
            painter.drawLine(piano_width/2, i*piano_keyboard_width+piano_keyboard_width/2 - y_scroll_offset, piano_width, i*piano_keyboard_width+piano_keyboard_width/2 - y_scroll_offset);
            wr = 0;
        } else {
            if(11-i%12 == 0) {
                painter.drawText(QPoint(piano_width/2, i*piano_keyboard_width + 10 - y_scroll_offset), QString(tr("C%1")).arg(oct));
                oct--;
            }
            if(wr) {
                painter.drawLine(0, i*piano_keyboard_width - y_scroll_offset, piano_width, i*piano_keyboard_width - y_scroll_offset);
            }
            wr++;
        }
    }
}

void PitchEditor::wheelEvent(QWheelEvent *ev)
{
    auto scry = ev->angleDelta().y();
    y_scroll_offset -= scry;
    if(y_scroll_offset < 0 || (y_scroll_offset+height())/piano_keyboard_width/12 > oct_max) y_scroll_offset += scry;
    ev->accept();
    update();
}
