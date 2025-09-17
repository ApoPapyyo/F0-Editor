#include "piano.hpp"
#include <QPainter>
#include <cmath>
#include <iostream>

namespace {
const int piano_structure[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const int piano_black_structure[12] = {0, 1, 0, 3, 0, 0, 1, 0, 2, 0, 3, 0};
}
Piano::Piano(const PitchEditor::Offset& offset_, const PitchEditor::Scale& scale_, const PitchEditor::Config& conf_, QWidget* parent)
    : QWidget{parent}
    , cursor()
    , synth(this)
    , offset(offset_)
    , scale(scale_)
    , conf(conf_)
{}

void Piano::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QColor white{255,255,255};
    const QColor wwhite{127, 127, 127};
    const QColor black{0, 0, 0};
    const QColor wblack{70, 70, 70};
    const int oct_range = conf.oct_max - conf.oct_min + 1;

    painter.setBrush(white);
    painter.drawRect(0, 0, width(), height());

    int last_k = -1;
    painter.setPen(black);
    for(int i = -scale.y*2; i < height(); i++) {
        const double key = 12.0 * oct_range - 0.5 - (double)(i + offset.y) / (double)scale.y;
        const int k = std::round(key);
        const int prev_k = k-1 < 0 ? (k+11)%12 : (k-1)%12;
        const int next_k = (k+1)%12;


        if(k < 0) break;//それ以上下は描画不要
        else if(k >= 12.0 * oct_range) continue;//範囲より高い鍵盤は飛ばす
        if(last_k == k) continue;

        if(!piano_structure[k%12]) {//白鍵
            if(cursor.click && k == std::round(cursor.pos - Note(Note::eNoteName::C, conf.oct_min, 0.0))) {
                painter.setBrush(wwhite);
            } else {
                painter.setBrush(white);
            }
            if(piano_structure[prev_k]) {//一個下のキーが黒鍵
                if(piano_structure[next_k]) {//一個上のキーが黒鍵
                    painter.drawRect(0, i - scale.y * (4 - piano_black_structure[next_k]) / 4, width(), scale.y * (4 - piano_black_structure[next_k]) / 4 + scale.y + scale.y * piano_black_structure[prev_k] / 4);
                    //std::cout << "上は黒鍵、下は黒鍵:" <<(Note(Note::eNoteName::C, conf.oct_min, 0.0) + key).toStr() << std::endl;
                } else {//一個上のキーが白鍵
                    painter.drawRect(0, i, width(), scale.y + scale.y * piano_black_structure[prev_k] / 4);
                    //std::cout << "上は白鍵、下は黒鍵:" <<(Note(Note::eNoteName::C, conf.oct_min, 0.0) + key).toStr() << std::endl;
                }
            } else {//一個下のキーが白鍵
                //一個上のキーが黒鍵
                painter.drawRect(0, i - scale.y * (4 - piano_black_structure[next_k]) / 4, width(), scale.y * (4 - piano_black_structure[next_k]) / 4 + scale.y);
                //std::cout << "上は黒鍵、下は白鍵:" <<(Note(Note::eNoteName::C, conf.oct_min, 0.0) + key).toStr() << std::endl;
            }

            if(k%12 == 0) painter.drawText(QPoint(width()-25, i + scale.y/2), (Note(Note::eNoteName::C, conf.oct_min, 0.0) + k).toStr(true).c_str());
        }

        last_k = k;
    }

    last_k = -1;
    for(int i = -scale.y*2; i < height(); i++) {
        const double key = 12.0 * oct_range - 0.5 - (double)(i + offset.y) / (double)scale.y;
        const int k = std::round(key);
        const int prev_k = k-1 < 0 ? (k+11)%12 : (k-1)%12;
        const int next_k = (k+1)%12;


        if(k < 0) break;//それ以上下は描画不要
        else if(k >= 12.0 * oct_range) continue;//範囲より高い鍵盤は飛ばす
        if(last_k == k) continue;

        if(piano_structure[k%12]) {//黒鍵
            if(cursor.click && k == std::round(cursor.pos - Note(Note::eNoteName::C, conf.oct_min, 0.0))) {
                painter.setBrush(wblack);
            } else {
                painter.setBrush(black);
            }
            painter.drawRect(0, i, width()/2, scale.y);
        }

        last_k = k;
    }
}

void Piano::mouseMoveEvent(QMouseEvent*)
{
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    const int oct_range = conf.oct_max - conf.oct_min + 1;
    double p = 12.0 * oct_range - 0.5 - (double)(pos.y() + offset.y) / (double)scale.y;
    cursor.pos = Note(Note::eNoteName::C, conf.oct_min, 0.0) + p;
    emit update_statusbar(cursor.pos.toStr().c_str());
    if(cursor.click) {
        freq_changed(cursor.pos.toHz(conf.A4));
        update();
    }
}

void Piano::mousePressEvent(QMouseEvent*)
{
    cursor.click = true;
    QPoint pos(QCursor::pos());
    pos = this->mapFromGlobal(pos);
    const int oct_range = conf.oct_max - conf.oct_min + 1;
    double p = 12.0 * oct_range - 0.5 - (double)(pos.y() + offset.y) / (double)scale.y;
    if(p < 0.0 || p >= oct_range*12) return;
    cursor.pos = Note(Note::eNoteName::C, conf.oct_min, 0.0) + p;
    freq_changed(cursor.pos.toHz(conf.A4));
    synth.play();
    update();
}

void Piano::mouseReleaseEvent(QMouseEvent*)
{
    cursor.click = false;
    freq_changed(0.0);
    synth.stop();
    update();
}

void Piano::freq_changed(double f)
{
    if(f > 0.0) {
        synth.setFreq(f, 0.01);
    }
}

