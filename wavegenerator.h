#pragma once

#include <QIODevice>
#include <QMutex>
#include <QAudioFormat>
#include <QList>
#include <QWaitCondition>

class WaveGenerator : public QIODevice {
    Q_OBJECT
public:
    explicit WaveGenerator(QObject* parent = nullptr);
    ~WaveGenerator();

    void start(const QList<double>& f0List, double fps);
    void startTone(double freq);
    void stop();
    void setFrequency(double freq);

    void setAudioFormat(const QAudioFormat& format);
    bool isSequential() const override { return true; }

signals:
    void cursorMoved(int frame);

protected:
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64) override { return 0; }

private:
    QMutex mutex_;
    QWaitCondition cond_;
    QList<double> f0List_;
    double fps_ = 100.0;
    QAudioFormat format_;
    bool toneMode_ = false;
    bool stopped_ = false;
    double currentFreq_ = 440.0;
    qint64 sampleIndex_ = 0;
    int currentFrame_ = 0;
    double phase_ = 0.0;

    double generateSample(double freq);
    double interpolatedFrequency(double timeInSec);

};
