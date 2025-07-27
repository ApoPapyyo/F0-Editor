#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QMutex>
#include <atomic>
#include <QTimer>

class SynthWorker : public QObject {
    Q_OBJECT
public:
    explicit SynthWorker(QObject* parent = nullptr);
    ~SynthWorker();

    void setupF0(const QList<double>& f0List, double fps);
    void play();
    void stop();
    void playTone(double freq);
    void setFrequency(double freq);

signals:
    void cursorMoved(int frame);

private:
    void initAudio();
    void generateNextBuffer(qint64 bytes);

    QAudioFormat m_format;
    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_audioDevice = nullptr;

    QList<double> m_f0List;
    double m_fps = 100.0;
    int m_currentFrame = 0;

    double m_sampleRate = 48000.0;
    double m_phase = 0.0;
    double m_currentFreq = 440.0;
    bool m_playingF0 = false;

    QMutex m_mutex;
    std::atomic<bool> m_stopRequested = false;
};
