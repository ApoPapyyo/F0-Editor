#pragma once

#include <QObject>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>

class SynthWorker;

class Synth : public QObject {
    Q_OBJECT
public:
    explicit Synth(QObject* parent = nullptr);
    ~Synth();

    void setF0(const QList<double> &f0List, double fps);
    void play();
    void stop();
    void playTone(double freq);
    void setFrequency(double freq);

signals:
    void cursorMoved(int frame);

private:
    QList<double> m_f0List;
    double m_fps = 100.0;

    QThread m_workerThread;
    SynthWorker* m_worker = nullptr;
};
