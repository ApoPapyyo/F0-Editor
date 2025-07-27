#include "synth.h"
#include "wavegenerator.h"

#include <QAudioDevice>
#include <QMediaDevices>

Synth::Synth(QObject* parent)
    : QObject(parent),
    generator_(new WaveGenerator)
{
    generator_->moveToThread(&workerThread_);
    connect(&workerThread_, &QThread::finished, generator_, &QObject::deleteLater);
    connect(generator_, &WaveGenerator::cursorMoved, this, &Synth::cursorMoved);
    workerThread_.start();
}

Synth::~Synth() {
    stop();
    workerThread_.quit();
    workerThread_.wait();
}

void Synth::setF0(const QList<double>& f0List, double fps) {
    f0List_ = f0List;
    fps_ = fps;
}

void Synth::play() {
    stop();

    QMetaObject::invokeMethod(generator_, [=]() {
        QAudioFormat format;
        format.setSampleRate(44100);
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);

        generator_->setAudioFormat(format);
        generator_->start(f0List_, fps_);

        QAudioDevice device = QMediaDevices::defaultAudioOutput();
        QAudioSink* sink = new QAudioSink(device, format, generator_);
        QObject::connect(sink, &QAudioSink::stateChanged, generator_, [sink](QAudio::State state) {
            if (state == QAudio::IdleState) {
                sink->stop();
                sink->deleteLater();
            }
        });
        sink->start(generator_);
    });
    isPlaying_ = true;
}

void Synth::stop() {
    if (generator_) {
        QMetaObject::invokeMethod(generator_, &WaveGenerator::stop, Qt::QueuedConnection);
    }
    isPlaying_ = false;
}

void Synth::playTone(double freq) {
    stop();

    QMetaObject::invokeMethod(generator_, [=]() {
        QAudioFormat format;
        format.setSampleRate(44100);
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);

        generator_->setAudioFormat(format);
        generator_->startTone(freq);

        QAudioDevice device = QMediaDevices::defaultAudioOutput();
        QAudioSink* sink = new QAudioSink(device, format, generator_);
        sink->start(generator_);
    });
}

void Synth::setFrequency(double freq) {
    if (generator_) {
        QMetaObject::invokeMethod(generator_, [=]() {
            generator_->setFrequency(freq);
        }, Qt::QueuedConnection);
    }
}
