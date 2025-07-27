#pragma once

#include <QObject>
#include <QList>
#include <QAudioSink>
#include <QIODevice>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>

class WaveGenerator;

class Synth : public QObject {
    Q_OBJECT
public:
    explicit Synth(QObject* parent = nullptr);
    ~Synth();

    void setF0(const QList<double>& f0List, double fps);
    void play();
    void stop();
    void playTone(double freq);
    void setFrequency(double freq);

signals:
    void cursorMoved(int frame);

private:
    QList<double> f0List_;
    double fps_ = 100.0;
    bool isPlaying_ = false;
    QThread workerThread_;
    WaveGenerator* generator_ = nullptr;
};
