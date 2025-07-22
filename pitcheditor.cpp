#include "pitcheditor.h"
#include <cmath>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>

const int PitchEditor::oct_max = 8;
PitchEditor::PitchEditor(QWidget *parent)
    : QWidget{parent}
    , piano_keyboard_width(20)
    , note_size(1.0)
    , now(Note(Note::C, 0.0, 4))
    , centrex(0)
    , centrey(Note(Note::C, 0.0, 4))
    , x_scroll_offset(0)
    , y_scroll_offset((Note(Note::B, 0.0, 8)-centrey)*piano_keyboard_width)
    , init(false)
    , f0()
    , mode(eMouseMode::Select)
    , selected()
    , lclick(false)
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
    painter.setPen(QColor(150, 150, 150));
    for (int i = y_scroll_offset; i < oct_max*12*piano_keyboard_width; i++) {
        if(i%piano_keyboard_width == 0) {
            painter.drawLine(QLine(0, i-y_scroll_offset, width(), i-y_scroll_offset));
        }
    }
    if(!init) {
        y_scroll_offset -= height()/2;
        init = true;
    }
    drawF0(painter);
    drawSelect(painter);
}

void PitchEditor::drawF0(QPainter &painter)
{
    int i(0), j(x_scroll_offset/note_size);
    painter.setBrush(QColor(255, 255, 255, 128));
    painter.drawRect(QRect(0, 0, std::min(f0.getDataSize()*note_size - x_scroll_offset, static_cast<double>(width())), height()));
    for(; i < width()+x_scroll_offset && j < f0.getDataSize(); j++) {
        i = j*note_size - x_scroll_offset;
        Note d(f0.getData(j)), d2(j+1 < f0.getDataSize() ? f0.getData(j+1) : Note());
        painter.setPen(QColor(200, 80, 0));
        painter.setBrush(QColor(200, 80, 0));
        if(d != Note() && d2 != Note()) {
            if(note_size > 4) painter.drawEllipse(QRect(i-2, soundPosition(d)-2, 4, 4));
            painter.drawLine(QLine(i, soundPosition(d), i+note_size, soundPosition(d2)));
        } else if(d != Note()) {
            painter.drawPoint(QPoint(i, soundPosition(d)));
            if(note_size > 4) painter.drawEllipse(QRect(i-2, soundPosition(d)-2, 4, 4));
        }
        painter.setPen(QColor(0, 0, 0));
        if(j%100 == 0 && note_size > 0.3 || j%1000 == 0 && note_size > 0.1 || j%2000 == 0 && note_size > 0.0333) {
            painter.drawLine(QLine(i, 0, i, height()));
            painter.drawText(QPoint(i+1, 20), tr("%1:%2").arg(j/100/60, 2, 10, QChar('0')).arg(j/100%60, 2, 10, QChar('0')));
        }
    }
}

void PitchEditor::drawSelect(QPainter &painter)
{
    painter.setBrush(QColor(0, 255, 255, 128));
    if(selected.seted()) painter.drawRect(QRect(selected.getStart()*note_size - x_scroll_offset, 0, selected.getEnd()*note_size - selected.getStart()*note_size, height()));
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
            for (y_scroll_offset = 0; centrey != mouseSound(QPoint(0, height()/2)); y_scroll_offset++);
        }
    } else {
        x_scroll_offset -= scrx;
        if(x_scroll_offset < 0) x_scroll_offset = 0;
        else if((x_scroll_offset+width()) > 10000) x_scroll_offset += scrx;

        y_scroll_offset -= scry;
        if(y_scroll_offset < 0) y_scroll_offset = 0;
        else if ((y_scroll_offset+height())/piano_keyboard_width/12 > oct_max) y_scroll_offset += scry;
        centrey = mouseSound(QPoint(0, height()/2));
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
    if(lclick) selected.setVar((x_scroll_offset + pos.x())/note_size);
    update();
}

void PitchEditor::mousePressEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    if (ev->button() == Qt::LeftButton) {
        lclick = true;
        switch(mode) {
        case eMouseMode::Select:
            if(!selected.seted()) selected.setRef((x_scroll_offset + pos.x()) / note_size);
            else selected.reset();
            break;
        default:
        }
    }
    update();
}

void PitchEditor::mouseReleaseEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    if (ev->button() == Qt::LeftButton) {
        lclick = false;
        switch(mode) {
        case eMouseMode::Select:
            if(!selected.seted()) selected.setVar((x_scroll_offset + pos.x()) / note_size);
            break;
        default:
        }
        centrex = selected.seted() ? (selected.getStart()+selected.getEnd())/2 : (x_scroll_offset + width()/2)/note_size;
    }
    update();
}

void PitchEditor::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape) {
        clicked_other();
    }
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
        centrex = selected.seted() ? (selected.getStart()+selected.getEnd())/2 : (x_scroll_offset + width()/2)/note_size;
        update();
    }
}

void PitchEditor::set_y_scroll_offset(int y)
{
    if(0 <= y && y+height() < oct_max*12*piano_keyboard_width) {
        y_scroll_offset = y;
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        centrey = mouseSound(QPoint(0, height()/2));
        update();
    }
}

double PitchEditor::get_x_zoom() const
{
    return note_size * 10.0;
}

int PitchEditor::get_y_zoom() const
{
    return piano_keyboard_width;
}

void PitchEditor::set_x_zoom(double x)
{
    x /= 10;
    if(0.01 <= x && x <= 100.0) {
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        note_size = x;
        for (x_scroll_offset = 0; centrex > (x_scroll_offset + width()/2)/note_size; x_scroll_offset++);
        update();
    }
}

void PitchEditor::set_y_zoom(int y)
{
    if(5 <= y && y <= 100) {
        piano_keyboard_width = y;
        emit scrolleds(x_scroll_offset, y_scroll_offset);
        for (y_scroll_offset = 0; centrey < mouseSound(QPoint(0, height()/2)); y_scroll_offset++);//ズームの中心を固定するため
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
        for(note_size = 0.0; note_size * f0.getDataSize() < width(); note_size+=0.01);
        update();
    }

}

void PitchEditor::close_f0()
{
    f0.closeF0();
    x_scroll_offset = 0;
    selected.reset();
    update();
}

void PitchEditor::clicked_other()
{
    selected.reset();
    update();
}

PitchEditor::area_t::area_t()
    : ref(-1)
    , var(-1)
{
}

int PitchEditor::area_t::getStart() const
{
    return ref < var ? ref : var;
}

int PitchEditor::area_t::getEnd() const
{
    return ref > var ? ref : var;
}

void PitchEditor::area_t::reset()
{
    ref = var = -1;
}

void PitchEditor::area_t::setRef(int n)
{
    if(n < 0) return;
    ref = n;
}

void PitchEditor::area_t::setVar(int n)
{
    if(n < 0) return;
    var = n;
}

int PitchEditor::area_t::getRef() const
{
    return ref;
}

int PitchEditor::area_t::getVar() const
{
    return var;
}

bool PitchEditor::area_t::seted() const
{
    return ref != -1 && var != -1;
}

QList<int> PitchEditor::area_t::getIndex() const
{
    if(!seted()) return QList<int>();
    QList<int> index;
    for(int i = getStart(); i < getEnd(); i++) index.append(i);
}
