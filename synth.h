#ifndef SYNTH_H
#define SYNTH_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QAudioDevice>
#include <QMediaDevices>
#include <cmath>

class AudioGenerator;

class Synth : public QObject {
    Q_OBJECT
    
public:
    Synth(QObject* parent = nullptr);
    ~Synth();
    
    void setF0(const QList<double> &freqs, double fps);
    void play(int frame_offset = 0);
    void stop();
    void playTone(double freq);
    void setFrequency(double freq);

signals:
    void cursorMoved(int frame);

private slots:
    void onFrameChanged(int frame);

private:
    void initializeAudio();
    void cleanup();
    
    QAudioFormat m_format;
    QAudioSink* m_audioSink;
    AudioGenerator* m_generator;
    QThread* m_audioThread;
    
    QList<double> m_f0Data;
    double m_fps;
    int m_currentFrame;
    
    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int BUFFER_SIZE = 4800; // 100ms at 48kHz
};

// 別スレッドで動作するオーディオ生成クラス
class AudioGenerator : public QIODevice {
    Q_OBJECT
    
public:
    AudioGenerator(QObject* parent = nullptr);
    
    void setF0Data(const QList<double>& data, double fps);
    void startPlayback(int frameOffset = 0);
    void startTone(double freq);
    void stopPlayback();
    void setCurrentFreq(double freq);
    
    // QIODevice interface
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;
    bool isSequential() const override { return true; }

signals:
    void frameChanged(int frame);

private:
    void generateSamples(float* buffer, int sampleCount);
    double generateSample(double freq);
    
    QMutex m_mutex;
    QList<double> m_f0Data;
    double m_fps;
    int m_currentFrame;
    int m_frameOffset;
    bool m_isPlaying;
    bool m_isTonePlaying;
    double m_currentFreq;
    double m_toneFreq;
    
    // 位相連続性のための変数
    double m_phase;
    
    // サンプル生成用
    int m_sampleIndex;
    int m_samplesPerFrame;
    
    static constexpr double PI = 3.14159265358979323846;
    static constexpr int SAMPLE_RATE = 48000;
};

#endif // SYNTH_H