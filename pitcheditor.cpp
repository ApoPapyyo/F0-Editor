#include "pitcheditor.hpp"
#include <iostream>
#include <cmath>
#include <QMouseEvent>

#define SAMPLE_RATE player.synth.getSampleRate()
#define CHANNEL player.synth.getChannelCount()
#define FORMAT_SIZE player.synth.getFormatSize()

namespace {
int countTrue(const QBitArray& b)
{
    int ret = 0;
    for(int i = 0; i < b.count(); i++) if(b.testBit(i)) ret++;
    return ret;
}

}

PitchEditor::PitchEditor(QWidget* parent)
    : QWidget{parent}
    , init{false}
    , offset{0, 0}
    , scale{1.0, 30}
    , mouse{eMouseMode::Select, QPoint{0, 0}, false, false, false}
    , log{this}
    , conf{9, -1, 440.0, ScaleConfig{eScaleMode::Time, 0.0, 0.0}}
    , player{Synth{}, 0, new QTimer(this)}
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    player.synth.setCurVar(&player.cur);
    connect(player.timer, &QTimer::timeout, this, [this]{
        update();
    });
}

int PitchEditor::notePos(const Note& n) const
{
    double r = Note(Note::eNoteName::B, conf.oct_max, 0.0) - n + 0.5;
    return r * scale.y - offset.y;
}

Note PitchEditor::posNote(int y) const
{
    double abs_y = y + offset.y;
    double r = abs_y / scale.y - 0.5;
    return Note{Note::eNoteName::B, conf.oct_max, 0.0} - r;
}

int PitchEditor::curPos(int cur) const
{
    return (int)std::round(scale.x * cur - offset.x);
}

int PitchEditor::posCur(int x) const
{
    int ret = (int)std::round((double)(offset.x + x) / scale.x);
    if(ret >= log.f0_data.getDataSize()) return log.f0_data.getDataSize()-1;
    else if(ret < 0) return 0;
    return ret;
}

PitchEditor::Offset PitchEditor::getScrollMax() const
{
    return Offset{(int)((double)log.f0_data.getDataSize()*scale.x) - width() < 0 ? 0 : (int)((double)log.f0_data.getDataSize()*scale.x) - width(), (conf.oct_max - conf.oct_min + 1)*12*scale.y - height() < 0 ? 0 : (conf.oct_max - conf.oct_min + 1)*12*scale.y - height()};
}

PitchEditor::Scale PitchEditor::getScaleMax() const
{
    return Scale{20.0, 70};
}

void PitchEditor::setScrollOffset(int x, int y)
{
    const auto max = getScrollMax();
    offset.x = x > max.x ? max.x : x;
    if(offset.x < 0) offset.x = 0;
    offset.y = y > max.y ? max.y : y;
    if(offset.y < 0) offset.y = 0;
}

void PitchEditor::addScrollOffset(int x, int y)
{
    setScrollOffset(offset.x + x, offset.y + y);
}

void PitchEditor::setScale(double x, int y)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    const auto h = posNote(height()/2);
    if(0 > pos.x() || pos.x() >= width()) pos.setX(width()/2);
    const auto w = posCur(pos.x());


    if(0.0 < x && x <= getScaleMax().x && log.f0_data.getDataSize() > 0) {
        if((double)width() / log.f0_data.getDataSize() <= x) scale.x = x;
        else scale.x = (double)width() / log.f0_data.getDataSize();
    }
    if(((double)height() / (double)((conf.oct_max - conf.oct_min + 1)*12.0)) <= y && y <= getScaleMax().y) scale.y = y;

    for(offset.y = 0; notePos(h) > height()/2; offset.y++);
    for(offset.x = 0; curPos(w) > pos.x(); offset.x++);
}

void PitchEditor::addScale(double x, int y)
{
    setScale(scale.x * pow(1.001, x), scale.y + y);
}

void PitchEditor::paintEvent(QPaintEvent*)
{
    QPainter painter{this};
    const QColor black{0, 0, 0};
    const QColor white{255, 255, 255};
    const QColor ggray{200, 200, 200};
    const QColor gray{150, 150, 150};
    const QColor red{255, 0, 0};
    const QColor blue{0, 0, 255};
    const QColor green{0, 255, 0};

    const bool editable = scale.x >= 5.0;

    if(!init) {//初期化
        init = true;
        for(offset.y = 0; notePos(Note{Note::eNoteName::C, 4, 0.0}) > height()/2; offset.y++);
    }

    //背景
    painter.setPen(gray);
    painter.setBrush(ggray);
    painter.drawRect(0, 0, width(), height());


    //横線
    int i;
    for(i = 0; (i + offset.y)%scale.y; i++);
    for(; i < height(); i+=scale.y) {
        painter.drawLine(0, i, width(), i);
    }

    //データ
    int last_cur = -1;
    for(int i = 0; i < width(); i++) {
        const int cur = posCur(i);


        if(cur >= log.f0_data.getDataSize()) break;
        if(cur == last_cur) continue;

        if(scale.x < 1.0) {
            for(int j = 0; j < (int)((double)(i + 1 + offset.x) / scale.x - cur); j++) {
                const auto d = log.f0_data.getData(cur+j);
                if(d == Note{}) continue;
                if(mouse.select.target.testBit(cur+j)) {
                    painter.setPen(red);
                } else {
                    painter.setPen(blue);
                }
                const auto dnext = log.f0_data.getData(cur+j+1);
                if(dnext != Note{}) painter.drawLine(i, notePos(d), i, notePos(dnext));
                else painter.drawPoint(QPoint(i, notePos(d)));
            }
        } else if(!editable) {
            const auto d = log.f0_data.getData(cur);
            if(d == Note{}) continue;
            if(mouse.select.target.testBit(cur)) {
                painter.setPen(red);
            } else {
                painter.setPen(blue);
            }
            const auto dnext = log.f0_data.getData(cur+1);
            if(dnext != Note{}) painter.drawLine(i, notePos(d), i+scale.x, notePos(dnext));
            else painter.drawPoint(QPoint(i, notePos(d)));
        } else {
            const auto d = log.f0_data.getData(cur);
            if(d == Note{}) continue;
            if(mouse.select.target.testBit(cur)) {
                painter.setBrush(red);
            } else {
                painter.setBrush(blue);
            }
            painter.drawEllipse(QRect(i-2, notePos(d)-2, 4, 4));
            const auto dnext = log.f0_data.getData(cur+1);
            if(dnext != Note{}) {
                if(mouse.select.target.testBit(cur+1)) {
                    painter.setPen(red);
                } else {
                    painter.setPen(blue);
                }
                painter.drawLine(i, notePos(d), i+scale.x, notePos(dnext));
            }
        }

        last_cur = cur;
    }

    //縦線
    if(!player.synth.isPlaying() && player.timer->isActive()) {
        player.timer->stop();
        player.cur = 0;
    }
    painter.setPen(green);
    const qsizetype cur = player.cur * log.f0_data.getFPS() / SAMPLE_RATE;
    if(player.synth.isPlaying()) {
        if(curPos(cur) < 0 || curPos(cur) >= width()) {
            for(offset.x = 0; curPos(cur) >= 0; offset.x++);
        }
        painter.drawLine(curPos(cur), 0, curPos(cur), height());
    }

    for(int i = 0; i < width(); i++) {

    }

    //選択
    if(mouse.mode == eMouseMode::Select) {
        if(editable) {

        }
    }
}

void PitchEditor::mouseMoveEvent(QMouseEvent*)
{
    //マウス座標
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    mouse.pos = pos;
    if(mouse.lclick || mouse.mclick || mouse.rclick) update();
}

void PitchEditor::mousePressEvent(QMouseEvent* e)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    mouse.pos = pos;
    if(e->button() == Qt::LeftButton) mouse.lclick = true;
    else if(e->button() == Qt::RightButton) mouse.rclick = true;
    else if(e->button() == Qt::MiddleButton) mouse.mclick = true;
    update();
}

void PitchEditor::mouseReleaseEvent(QMouseEvent* e)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    mouse.pos = pos;
    if(e->button() == Qt::LeftButton) mouse.lclick = false;
    else if(e->button() == Qt::RightButton) mouse.rclick = false;
    else if(e->button() == Qt::MiddleButton) mouse.mclick = false;
    update();
}

void PitchEditor::keyPressEvent(QKeyEvent*)
{

}

void PitchEditor::closeEvent(QCloseEvent*)
{
    log.f0_data.closeF0();
}

void PitchEditor::openF0(const QString& path)
{
    QFileInfo tmp{path};
    log.f0_data.openF0(tmp);
    mouse.select.target.resize(log.f0_data.getDataSize());
    scale.x = (double)width() / log.f0_data.getDataSize();
    update();
}

void PitchEditor::closeF0()
{
    log.f0_data.closeF0();
    offset.x = 0;
    player.cur = 0;
    player.synth.setFreq(0.0);
    mouse.select.target.clear();
    update();
}

void PitchEditor::saveF0()
{
    log.f0_data.saveF0();
    update();
}

void PitchEditor::saveF0as(const QString& path)
{
    QFileInfo tmp{path};
    log.f0_data.saveF0as(tmp);
    update();
}

void PitchEditor::play()
{
    player.synth.setData(log.f0_data.getFreq(conf.A4), 1.0 / (double)log.f0_data.getFPS());
    player.timer->start(20);
    player.synth.play();
}

void PitchEditor::stop()
{
    player.timer->stop();
    player.synth.stop();
    player.cur = 0;
    update();
}

void PitchEditor::pause()
{
    player.timer->stop();
    player.synth.stop();
    update();
}


PitchEditor::Area::Area()
    : ref(nullptr)
    , var(nullptr)
{}

PitchEditor::Area::Area(const QPoint& ref, const QPoint& var)
    : ref(nullptr)
    , var(nullptr)
{
    if(!ref.isNull()) {
        this->ref = new QPoint{ref};
        if(!var.isNull()) this->var = new QPoint{var};
    }
}

PitchEditor::Area::~Area()
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

QPoint PitchEditor::Area::getRef() const
{
    return ref ? *ref : QPoint{};
}

QPoint PitchEditor::Area::getVar() const
{
    return var ? *var : QPoint{};
}

QRect PitchEditor::Area::getRect() const
{
    return ref && var ? QRect(ref->x(), ref->y(), var->x() - ref->x(), var->y() - ref->y()) : QRect{};
}

int PitchEditor::Area::getWidth() const
{
    return ref && var ? var->x() - ref->x() : 0;
}

int PitchEditor::Area::getHeight() const
{
    return ref && var ? var->y() - ref->y() : 0;
}

void PitchEditor::Area::reset()
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

void PitchEditor::Area::setRef(const QPoint& ref)
{
    if(!ref.isNull() && !this->ref) {
        this->ref = new QPoint{ref};
    }
}

void PitchEditor::Area::setVar(const QPoint& var)
{
    if(!var.isNull()) {
        if(!this->var) this->var = new QPoint{var};
        else *this->var = var;
    }
}

bool PitchEditor::Area::isSet() const
{
    return ref && var;
}

bool PitchEditor::Area::isInside(const QPoint& n) const
{
    if(n.isNull()) return false;

    return getRect().contains(n);
}

bool PitchEditor::Area::isEmpty() const
{
    return !ref && !var;
}

PitchEditor::OprLog::OprLog(QWidget* parent)
    : index{0}
    , data{}
    , f0_data{parent}
{}

QString PitchEditor::OprLog::getActionName() const
{
    if(data.size() <= 0) return tr("空");
    QString ret;
    for(int i = data.size()-1; i >= 0; i--) {
        switch(data[i].action) {
        case eAction::PitchShift:
            ret += tr("%1個のフレームのピッチシフト%2").arg(countTrue(data[i].target)).arg(index == i ? "←" : "");
            break;

        case eAction::PitchDel:
            ret += tr("%1個のフレームのピッチを削除%2").arg(countTrue(data[i].target)).arg(index == i ? "←" : "");
            break;

        case eAction::PitchMod:
            ret += tr("%1個のフレームのピッチを変更%2").arg(countTrue(data[i].target)).arg(index == i ? "←" : "");
            break;

        default:
            break;
        }
        ret += "\n";
    }
    return ret;
}

void PitchEditor::OprLog::pushShift(const QBitArray& target, double diff)
{
    if(index < data.size()) {
        int n = data.size() - index;
        data.erase(data.end() - n, data.end());
    }

    for(int i = 0; i < f0_data.getDataSize(); i++) {
        if(target.testBit(i)) {
            f0_data.setData(i, f0_data.getData(i) + diff);
        }
    }

    Data d;
    d.action = eAction::PitchShift;
    d.target = target;
    d.diff = diff;
    data.append(std::move(d));
    index++;
}

void PitchEditor::OprLog::pushWrite(size_t target_idx, Note& now)
{
    if(index < data.size()) {
        int n = data.size() - index;
        data.erase(data.end() - n, data.end());
    }

    QBitArray tmp(f0_data.getDataSize(), false);
    tmp.setBit(target_idx, true);
    Data d;
    d.action = eAction::PitchMod;
    d.target = tmp;
    d.now.append(now);
    d.past.append(f0_data.getData(target_idx));
    f0_data.setData(target_idx, now);
    data.append(std::move(d));
    index++;
}

void PitchEditor::OprLog::pushWrite(const QBitArray& target, QList<Note>& now)
{
    if(index < data.size()) {
        int n = data.size() - index;
        data.erase(data.end() - n, data.end());
    }

    Data d;
    d.action = eAction::PitchMod;
    d.target = target;
    d.now = std::move(now);
    QList<Note> tmp(d.now.size());
    int j = 0;
    for(int i = 0; i < f0_data.getDataSize(); i++) {
        if(target.testBit(i)) {
            tmp.append(f0_data.getData(i));
            f0_data.setData(i, d.now[j++]);
        }
    }
    d.past = std::move(tmp);
    data.append(std::move(d));
    index++;
}

void PitchEditor::OprLog::pushErase(size_t target_idx)
{
    if(index < data.size()) {
        int n = data.size() - index;
        data.erase(data.end() - n, data.end());
    }

    Data d;
    d.action = eAction::PitchDel;
    QBitArray tmp(f0_data.getDataSize(), false);
    tmp.setBit(target_idx, true);
    d.target = tmp;
    d.past.append(f0_data.getData(target_idx));
    f0_data.setData(target_idx, Note{});
    data.append(std::move(d));
    index++;
}

void PitchEditor::OprLog::pushErase(const QBitArray& target)
{
    if(index < data.size()) {
        int n = data.size() - index;
        data.erase(data.end() - n, data.end());
    }

    Data d;
    d.action = eAction::PitchDel;
    d.target = target;
    for(int i = 0; i < f0_data.getDataSize(); i++) {
        if(target.testBit(i)) {
            d.past.append(f0_data.getData(i));
            f0_data.setData(i, Note{});
        }
    }
    data.append(std::move(d));
    index++;
}

void PitchEditor::OprLog::undo()
{
    if(index == 0) return;

    Data d(data.takeAt(index-1));
    switch(d.action) {
    case eAction::PitchShift:
        {
            double diff = d.diff;
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, f0_data.getData(i) - diff);
                }
            }
            index--;
        }
        break;
    case eAction::PitchMod:
        if(d.past.size() > 0) {
            auto& past = d.past;
            int j = 0;
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, past[j++]);
                }
            }
            index--;
        }
        break;
    case eAction::PitchDel:
        if(d.past.size() > 0) {
            auto& past = d.past;
            int j = 0;
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, past[j++]);
                }
            }
            index--;
        }
        break;
    default:
        break;
    }
    data.insert(index, std::move(d));
}

void PitchEditor::OprLog::redo()
{
    if(data.size() - index <= 0) return;

    Data d = data.takeAt(index);
    switch(d.action) {
    case eAction::PitchShift:
        {
            double diff = d.diff;
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, f0_data.getData(i) + diff);
                }
            }
            index++;
        }
        break;
    case eAction::PitchMod:
        if(d.now.size() > 0) {
            auto& now(d.now);
            int j = 0;
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, now[j++]);
                }
            }
            index++;
        }
        break;
    case eAction::PitchDel:
        if(d.past.size() > 0) {
            for(int i = 0; i < f0_data.getDataSize(); i++) {
                if(d.target.testBit(i)) {
                    f0_data.setData(i, Note{});
                }
            }
            index++;
        }
        break;
    default:
        break;
    }
    data.insert(index-1, std::move(d));
}

bool PitchEditor::OprLog::ableToUndo() const
{
    return index > 0;
}

bool PitchEditor::OprLog::ableToRedo() const
{
    return index < data.size();
}
