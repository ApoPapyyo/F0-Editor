#include "pitcheditor.h"
#include <cmath>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <cstdlib>
#define SCROLL_OFFSET QPoint(-x_scroll_offset, -y_scroll_offset)

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
    , rectselect()
    , selected()
    , modlog()
    , mouse(eMouseAction::None)
    , draggedid(0)
    , mousexy()
    , sel(-1)
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
    if(!f0.getDataSize()) {
        painter.drawRect(QRect(0, 0, width(), height()));
    }
    painter.setPen(QColor(150, 150, 150));
    for (int i = y_scroll_offset; i < oct_max*12*piano_keyboard_width; i++) {
        if(i%piano_keyboard_width == 0) {
            painter.drawLine(QLine(0, i-y_scroll_offset, width(), i-y_scroll_offset));
        }
    }
    if(!init) {
        y_scroll_offset -= height()/2;
        emit titlechange("");
        init = true;
    }
    drawF0(painter);
    drawSelect(painter);

    int max = 0;
    if(!selected.isEmpty()) for(auto i: selected) if(i > max) max = i;
    int min = max;
    if(!selected.isEmpty()) for(auto i: selected) if(i < min) min = i;
    centrex = !selected.isEmpty() ? (min + max)/2 : (x_scroll_offset + width()/2)/note_size;
    max = 0;
    if(!selected.isEmpty()) for(auto i: selected) if(soundPosition(f0.getData(i)) > max) max = soundPosition(f0.getData(i));
    min = max;
    if(!selected.isEmpty()) for(auto i: selected) if(soundPosition(f0.getData(i)) < min) min = soundPosition(f0.getData(i));
    centrey = mouseSound(!selected.isEmpty() ? QPoint(0, (min + max)/2) : QPoint(0, height()/2));
    if(f0.isChanged()) emit titlechange(QString("*") + f0.getFileName());
}

void PitchEditor::drawF0(QPainter &painter)
{
    int i(0), j(x_scroll_offset/note_size);
    if(!f0.getDataSize()) return;
    painter.setBrush(QColor(255, 255, 255, 128));
    painter.drawRect(QRect(0, 0, std::min(f0.getDataSize()*note_size - x_scroll_offset, static_cast<double>(width())), height()));
    for(; i < width()+x_scroll_offset && j < f0.getDataSize(); j++) {
        i = j*note_size - x_scroll_offset;
        Note d(f0.getData(j)), d2(j+1 < f0.getDataSize() ? f0.getData(j+1) : Note());
        if(!selected.contains(j)) painter.setPen(QColor(200, 80, 0));
        else painter.setPen(QColor(0, 100, 200));
        if(!selected.contains(j)) painter.setBrush(QColor(200, 80, 0));
        else painter.setBrush(QColor(0, 100, 200));
        if(d != Note() && d2 != Note()) {
            if(note_size > 4) painter.drawEllipse(QRect(i-2, soundPosition(d)-2, 4, 4));
            painter.drawLine(QLine(i, soundPosition(d), i+note_size, soundPosition(d2)));
        } else if(d != Note()) {
            painter.drawPoint(QPoint(i, soundPosition(d)));
            if(note_size > 4) painter.drawEllipse(QRect(i-2, soundPosition(d)-2, 4, 4));
        }
        painter.setPen(QColor(0, 0, 0));
        if(j%f0.getFPS() == 0 && note_size > 0.3 || j%(f0.getFPS()*10) == 0 && note_size > 0.1 || j%(f0.getFPS()*20) == 0 && note_size > 0.0333) {
            painter.drawLine(QLine(i, 0, i, height()));
            painter.drawText(QPoint(i+1, 20), tr("%1:%2").arg(j/f0.getFPS()/60, 2, 10, QChar('0')).arg(j/f0.getFPS()%60, 2, 10, QChar('0')));
        }
    }

}

void PitchEditor::drawSelect(QPainter &painter)
{
    painter.setBrush(QColor(0, 0, 0, 60));
    if(rectselect.seted()) painter.drawRect(rectselect.getRect().translated(SCROLL_OFFSET));
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

Note PitchEditor::mouseSound(int p) const
{
    int y(y_scroll_offset+p-piano_keyboard_width/2);
    double y_ = static_cast<double>(y)/piano_keyboard_width;
    y_ = 12*oct_max - y_;
    return Note(Note::B, y_, 0);
}

Note PitchEditor::pos2sound(QPoint p) const
{
    int y(p.y() - piano_keyboard_width/2);
    double y_ = static_cast<double>(y)/piano_keyboard_width;
    y_ = 12*oct_max - y_;
    return Note(Note::B, y_, 0);
}

Note PitchEditor::pos2sound(int p) const
{
    int y(p - piano_keyboard_width/2);
    double y_ = static_cast<double>(y)/piano_keyboard_width;
    y_ = 12*oct_max - y_;
    return Note(Note::B, y_, 0);
}


void PitchEditor::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    now = mouseSound(pos);
    if(lclick) {
        if(mode == eMouseMode::Select) {
            if(rectselect.seted()) {
                rectselect.setVar(pos - SCROLL_OFFSET);
            } else if(mousexy != pos) {//動いたらピッチシフトの操作で確定
                mousexy = pos;
                draggedid = sel;
                mouse = eMouseAction::PitchDrag;
                for(auto i: selected) {
                    if(draggedid == i) {
                        Note tmp(f0.getData(i));
                        f0.setData(i, mouseSound(pos));
                        double diff(f0.getData(i) - tmp);
                        for(auto j: selected) {
                            if(draggedid != j) f0.setData(j, f0.getData(j) + diff);
                        }
                        break;
                    }
                }
            }
        } else if(mode == eMouseMode::Write) {
            if(note_size > 4) {
                f0.setData((x_scroll_offset + pos.x())/note_size, mouseSound(pos.y()));
            }
        }
    }
    emit mouseMoved(tr("%1Hzは%2(%3セント). A4=440Hzとした%2は%4Hz. C0との音程は%5.").arg(now.toHz()).arg(now.toStr()).arg(now.getCent()*100).arg(Note(now.getName(), 0.0, now.getOct()).toHz()).arg(now-Note(Note::C, 0.0, 0)));
    update();
}

void PitchEditor::mousePressEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    lclick = true;
    bool tch(false);
    if (ev->button() == Qt::LeftButton) {
        switch(mode) {
        case eMouseMode::Select:
            if(note_size > 4) {
                for(int i = x_scroll_offset / note_size; i*note_size < x_scroll_offset+width(); i++) {
                    if(i*note_size-x_scroll_offset - 2 <= pos.x() && pos.x() <= i*note_size-x_scroll_offset + 2
                        && soundPosition(f0.getData(i)) - 2 <= pos.y() && pos.y() <= soundPosition(f0.getData(i)) + 2) {
                        if(! selected.contains(i)) {
                            if(!(ev->modifiers() & Qt::ControlModifier)) selected.clear();
                            selected.append(i);
                            sel = -1;

                        } else {
                            mousexy = pos;
                            //動くか動かないかは別で判定
                            //動いたらピッチシフト、動かなかったら、選択の反転
                            sel = i;
                        }
                        tch = true;
                        break;
                    }
                }
            }
            if(tch) break;
            if(!(ev->modifiers() & Qt::ControlModifier)) selected.clear();
            rectselect.setRef(pos - SCROLL_OFFSET);
            rectselect.setVar(pos - SCROLL_OFFSET);
            break;
        case eMouseMode::Write:
            if(note_size > 4) {
                f0.setData((x_scroll_offset + pos.x())/note_size, mouseSound(pos.y()));
            }
            break;
        default:
            break;
        }
    }
    update();
}

void PitchEditor::mouseReleaseEvent(QMouseEvent *ev)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    lclick = false;
    if (ev->button() == Qt::LeftButton) {
        switch(mode) {
        case eMouseMode::Select:
            if(rectselect.seted()) {
                rectselect.setVar(pos - SCROLL_OFFSET);
                Note top(pos2sound(rectselect.getRect().top())), bottom(pos2sound(rectselect.getRect().bottom()));
                if(f0.getDataSize()) for(int i = rectselect.getRect().left()/note_size; i < rectselect.getRect().right()/note_size; i++) {
                    Note d(f0.getData(i));
                    if(bottom <= d && d <= top) selected.append(i);
                }
                rectselect.reset();
            } else if(mouse == eMouseAction::PitchDrag) {
                mouse = eMouseAction::None;
            } else if(mousexy == pos){//選択反転
                int i(sel);
                for(auto j = selected.begin(); j != selected.end(); j++) {
                    if(*j == i) {
                        selected.erase(j);
                        break;
                    }
                }
                sel = -1;
            }
            break;
        default:
            break;
        }
    }
    update();
}

void PitchEditor::keyPressEvent(QKeyEvent *ev)
{
    auto ctrl = [&]() -> bool {
        if(ev->modifiers() & Qt::ControlModifier) return true;
        return false;
    };
    auto alt = [&]() -> bool {
        if(ev->modifiers() & Qt::AltModifier) return true;
        return false;
    };
    int n(ev->key());
    switch(n) {
    case Qt::Key_Escape:
        if(!ctrl() && !alt()) clicked_other();
        break;
    case Qt::Key_Up:
        if(!ctrl() && !alt()) {
            set_y_scroll_offset(get_y_scroll_offset() - 10);
        } else if(!ctrl() && alt()) {
            if(!selected.empty()) {
                for(auto i: selected) {
                    f0.setData(i, f0.getData(i) + 1);
                }
                update();
            }
        }
        break;
    case Qt::Key_Down:
        if(!ctrl() && !alt()) {
            set_y_scroll_offset(get_y_scroll_offset() + 10);
        } else if(!ctrl() && alt()) {
            if(!selected.empty()) {
                for(auto i: selected) {
                    f0.setData(i, f0.getData(i) - 1);
                }
                update();
            }
        }
        break;
    case Qt::Key_Left:
        if(!ctrl() && !alt()) {
            set_x_scroll_offset(get_x_scroll_offset() - 5);
        }
        break;
    case Qt::Key_Right:
        if(!ctrl() && !alt()) {
            set_x_scroll_offset(get_x_scroll_offset() + 5);
        }
        break;
    case Qt::Key_Backspace:
        if(!selected.empty()) {
            for(auto i: selected) {
                f0.setData(i, Note());
            }
            selected.clear();
            update();
        }
        break;
    default:
        break;
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
    int x_max(f0.getDataSize()*note_size - width());
    if(0 <= x && x < x_max) {
        x_scroll_offset = x;
    } else {
        x_scroll_offset = x < 0 ? 0 : x_max;
    }
    emit scrolleds(x_scroll_offset, y_scroll_offset);
    update();
}

void PitchEditor::set_y_scroll_offset(int y)
{
    int y_max(oct_max*12*piano_keyboard_width - height());
    if(0 <= y && y < y_max) {
        y_scroll_offset = y;
    } else {
        y_scroll_offset = y < 0 ? 0 : y_max;
    }
    emit scrolleds(x_scroll_offset, y_scroll_offset);
    update();
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
    if (!f0.openF0(fn)) {
        for(note_size = 0.0; note_size * f0.getDataSize() < width(); note_size+=0.01);
        emit titlechange(QString(f0.isChanged() ? "*" : "") + f0.getFileName());
        update();
    }

}

void PitchEditor::close_f0()
{
    if(f0.isChanged()) {
        int reply(question());

        if (reply == 0) {
            save_f0();
        } else if(reply == 2) {
            update();
            return;
        };
    }
    f0.closeF0();
    x_scroll_offset = 0;
    rectselect.reset();
    selected.clear();
    emit titlechange(QString(f0.isChanged() ? "*" : "") + f0.getFileName());
    update();
}

void PitchEditor::closeEvent(QCloseEvent *ev)
{
    if(f0.isChanged()) {
        int reply(question());

        if (reply == 0) {
            save_f0();
        } else if(reply == 2) {
            update();
            ev->ignore();
            return;
        };
    }
    f0.closeF0();
    x_scroll_offset = 0;
    rectselect.reset();
    selected.clear();
    ev->accept();
}

int PitchEditor::question() const
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr,
                                  tr("確認"),
                                  tr("ファイルが変更されています。保存しますか？"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) return 0;
    else if(reply == QMessageBox::No) return 1;
    else return 2;
}

void PitchEditor::save_f0()
{
    if(f0.getDataSize() == 0) return;
    if(f0.saveF0()) return;
    emit titlechange(QString("") + f0.getFileName());
}

void PitchEditor::save_f0_as()
{
    if(f0.getDataSize() == 0) return;
    QFileInfo fn(QFileDialog::getSaveFileName(
        this,
        tr("ファイルを保存"),
        "",
        tr("RVC用F0ファイル (*.csv);;NEUTRINO用F0ファイル (*.f0)")
        ));
    if(fn.suffix().toLower() != "csv" && fn.suffix().toLower() != "f0") {
        fn.setFile(fn.filePath() + ".csv");
    }
    if(f0.saveF0as(fn)) return;
    emit titlechange(QString("") + f0.getFileName());
}

void PitchEditor::clicked_other()
{
    rectselect.reset();
    selected.clear();
    update();
}

void PitchEditor::setMode(eMouseMode mode)
{
    this->mode = mode;
    update();
}

PitchEditor::eMouseMode PitchEditor::getMode() const
{
    return mode;
}

PitchEditor::Area::Area()
    : ref(-1)
    , var(-1)
{}

PitchEditor::Area::Area(int ref, int var)
    : ref(ref >= 0 ? ref : -1)
    , var(var >= 0 ? var : -1)
{}

int PitchEditor::Area::getStart() const
{
    return ref < var ? ref : var;
}

int PitchEditor::Area::getEnd() const
{
    return ref > var ? ref : var;
}

void PitchEditor::Area::reset()
{
    ref = var = -1;
}

void PitchEditor::Area::setRef(int n)
{
    if(n < 0) return;
    ref = n;
}

void PitchEditor::Area::setVar(int n)
{
    if(n < 0) return;
    var = n;
}

int PitchEditor::Area::getRef() const
{
    return ref;
}

int PitchEditor::Area::getVar() const
{
    return var;
}

bool PitchEditor::Area::seted() const
{
    return ref != -1 && var != -1;
}

QList<int> PitchEditor::Area::getIndex() const
{
    if(!seted()) return QList<int>();
    QList<int> index;
    for(int i = getStart(); i < getEnd(); i++) index.append(i);
    return index;
}

PitchEditor::Area2::Area2()
    : ref(nullptr)
    , var(nullptr)
{}

PitchEditor::Area2::Area2(const QPoint &ref, const QPoint &var)
    : ref(!ref.isNull() ? new QPoint(ref) : nullptr)
    , var(!var.isNull() ? new QPoint(var) : nullptr)
{}

QPoint PitchEditor::Area2::getRef() const
{
    if(ref == nullptr) return QPoint();
    return *ref;
}

QPoint PitchEditor::Area2::getVar() const
{
    if(var == nullptr) return QPoint();
    return *var;
}

QRect PitchEditor::Area2::getRect() const
{
    if(!ref || !var) return QRect();
    int dx(ref->x() < var->x() ? var->x() - ref->x() : ref->x() - var->x()),
        dy(ref->y() < var->y() ? var->y() - ref->y() : ref->y() - var->y());
    int sx(ref->x() < var->x() ? ref->x() : var->x()),
        sy(ref->y() < var->y() ? ref->y() : var->y());
    return QRect(sx, sy, dx, dy);
}

int PitchEditor::Area2::getWidth() const
{
    if(!ref || !var) return 0;
    return ref->x() < var->x() ? var->x() - ref->x() : ref->x() - var->x();
}

int PitchEditor::Area2::getHeight() const
{
    if(!ref || !var) return 0;
    return ref->y() < var->y() ? var->y() - ref->y() : ref->y() - var->y();
}

void PitchEditor::Area2::reset()
{
    if(ref) {
        delete ref;
        ref = nullptr;
    }
    if(var) {
        delete var;
        var = nullptr;
    }
}

void PitchEditor::Area2::setRef(const QPoint &_ref)
{
    if(!_ref.isNull() && !ref) {
        ref = new QPoint();
        *ref = _ref;
    }
}

void PitchEditor::Area2::setVar(const QPoint &_var)
{
    if(!_var.isNull()) {
        if(!var) var = new QPoint();
        *var = _var;
    }
}

bool PitchEditor::Area2::seted() const
{
    return ref && var;
}

bool PitchEditor::Area2::isin(const QPoint &n) const
{
    if(n.isNull()) return false;

    return getRect().contains(n);
}

bool PitchEditor::Area2::isempty() const
{
    return !ref && !var;
}
