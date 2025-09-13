#include "midieditor.hpp"
#include <QWheelEvent>
#include <cmath>
#include <iostream>

const int bar2val = 10;


MidiEditor::MidiEditor(QWidget *parent)
    : QWidget{parent}
    , _lay(new QGridLayout(this))
    , _sp(new QSplitter(Qt::Horizontal, this))
    , _pe(new PitchEditor(this))
    , _piano(new Piano(_pe->offset, _pe->scale, _pe->conf, this))
    , _scrx(new QScrollBar(Qt::Horizontal, this))
    , _scry(new QScrollBar(Qt::Vertical, this))
    , x_zoom_offset(1)
    , y_zoom_offset(10)
    , x_zoom_max(100)
    , y_zoom_max(100)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    _sp->addWidget(_piano);
    _sp->addWidget(_pe);
    _sp->setSizes(QList({static_cast<int>(width()/6.0), static_cast<int>(width()/6.0*5.0)}));
    _lay->addWidget(_sp, 0, 0, 1, 1);
    _lay->addWidget(_scry, 0, 1, 1, 1);
    _lay->addWidget(_scrx, 1, 0, 1, 1);
    _scrx->setRange(0, _pe->getScrollMax().x/bar2val);
    _scry->setRange(0, _pe->getScrollMax().y/bar2val);
    connect(_scrx, &QScrollBar::sliderMoved, this, &MidiEditor::scrollEventX);
    connect(_scry, &QScrollBar::sliderMoved, this, &MidiEditor::scrollEventY);
}

MidiEditor::~MidiEditor()
{
    if(_scry) delete _scry;
    if(_scrx) delete _scrx;
    if(_piano) delete _piano;
    if(_pe) delete _pe;
    if(_lay) delete _lay;
}


void MidiEditor::wheelEvent(QWheelEvent *ev)
{
    Qt::KeyboardModifiers mods = ev->modifiers();
    auto scrx = ev->angleDelta().x(), scry = ev->angleDelta().y();
    if (mods & Qt::ShiftModifier) {
        auto tmp = scrx;
        scrx = scry;
        scry = tmp;
    }
    if (mods & Qt::ControlModifier) {
        _pe->addScale(-scrx/120,-scry/10);
        _scrx->setRange(0, _pe->getScrollMax().x/bar2val);
        _scry->setRange(0, _pe->getScrollMax().y/bar2val);
        _scrx->setValue(_pe->offset.x/10);
        _scry->setValue(_pe->offset.y/10);
    } else {
        _pe->addScrollOffset(-scrx, -scry);
        _scrx->setValue(_pe->offset.x/10);
        _scry->setValue(_pe->offset.y/10);
    }
    _piano->update();
    _pe->update();
    ev->accept();
}

void MidiEditor::paintEvent(QPaintEvent *ev)
{
    _scrx->setRange(0, _pe->getScrollMax().x/bar2val);
    _scry->setRange(0, _pe->getScrollMax().y/bar2val);
    _scrx->setValue(_pe->offset.x/bar2val);
    _scry->setValue(_pe->offset.y/bar2val);
}

void MidiEditor::scrollEventX(int x)
{
    _pe->setScrollOffset(x*bar2val, _pe->offset.y);
    _pe->update();
}

void MidiEditor::scrollEventY(int y)
{
    _pe->setScrollOffset(_pe->offset.x, y*bar2val);
    _piano->update();
    _pe->update();
}

PitchEditor *MidiEditor::get_pe() const
{
    return _pe;
}

Piano *MidiEditor::get_piano() const
{
    return _piano;
}
