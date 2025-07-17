#include "piano.h"
#include <QPainter>
#include <cmath>
const int piano_structure[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const int piano_black_structure[12] = {0, 3, 0, 1, 0, 0, 3, 0, 2, 0, 1, 0};
Piano::Piano(PitchEditor *pe, QWidget *parent)
    : QWidget{parent}
    , pe(pe)
    , keysize(pe->piano_keyboard_width)
{
}

void Piano::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);

    int wr(0), oct(PitchEditor::oct_max - pe->y_scroll_offset/keysize/12);
    int i;
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRect(QRect(0, 0, width(), height()));
    painter.setBrush(QColor(0, 0, 0));
    painter.drawLine(width(), 0, width(), height());
    for(i = pe->y_scroll_offset/keysize; i*keysize - pe->y_scroll_offset < height() && oct > -1; i++) {
        if (piano_structure[11-i%12]) {
            painter.drawRect(QRect(0, i*keysize - pe->y_scroll_offset, width()/2, keysize));
            //painter.drawLine(width()/2, i*keysize+keysize/2 - pe->y_scroll_offset, width(), i*keysize+keysize/2 - pe->y_scroll_offset);
            int y(std::round(static_cast<double>(i*keysize) + static_cast<double>(4-piano_black_structure[11-i%12])/4.0*keysize));
            painter.drawLine(
                width()/2,
                y - pe->y_scroll_offset,
                width(),
                y - pe->y_scroll_offset
                );
            wr = 0;
        } else {
            if(11-i%12 == 0) {
                painter.drawText(QPoint(width()/2, i*keysize + 10 - pe->y_scroll_offset), QString(tr("C%1")).arg(oct));
                oct--;
            }
            if(wr) {
                painter.drawLine(0, i*keysize - pe->y_scroll_offset, width(), i*keysize - pe->y_scroll_offset);
            }
            wr++;
        }
    }
    if(oct == -1) {
        painter.drawLine(0, i*keysize - pe->y_scroll_offset, width(), i*keysize - pe->y_scroll_offset);
    }
}

void Piano::scrolled(int x, int y)
{
    update();
}
