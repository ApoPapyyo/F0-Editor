#include "wavegenerator.h"
#include <QtMath>

WaveGenerator::WaveGenerator(QObject* parent)
    : QIODevice(parent)
{}

WaveGenerator::~WaveGenerator() {
    stop();
}

void WaveGenerator::setAudioFormat(const QAudioFormat& format) {
    format_ = format;
}

void WaveGenerator::start(const QList<double>& f0List, double fps) {
    QMutexLocker locker(&mutex_);
    f0List_ = f0List;
    fps_ = fps;
    toneMode_ = false;
    stopped_ = false;
    sampleIndex_ = 0;
    currentFrame_ = 0;
    open(QIODevice::ReadOnly);
}

void WaveGenerator::startTone(double freq) {
    QMutexLocker locker(&mutex_);
    toneMode_ = true;
    currentFreq_ = freq;
    stopped_ = false;
    sampleIndex_ = 0;
    open(QIODevice::ReadOnly);
}

void WaveGenerator::stop() {
    QMutexLocker locker(&mutex_);
    stopped_ = true;
    close();
}

void WaveGenerator::setFrequency(double freq) {
    QMutexLocker locker(&mutex_);
    currentFreq_ = freq;
}
int bytesPerSample(QAudioFormat::SampleFormat fmt) {
    switch (fmt) {
    case QAudioFormat::UInt8: return 1;
    case QAudioFormat::Int16: return 2;
    case QAudioFormat::Int32:
    case QAudioFormat::Float: return 4;
    default: return 0;
    }
}
qint64 WaveGenerator::readData(char* data, qint64 maxlen) {
    QMutexLocker locker(&mutex_);
    if (stopped_ || !format_.isValid()) return 0;

    const int sampleRate = format_.sampleRate();
    const int channelBytes = bytesPerSample(format_.sampleFormat());
    if (channelBytes <= 0) return 0; // ← これが必須！！
    const int frameSize = channelBytes * format_.channelCount();
    const int numSamples = maxlen / frameSize;

    qint16* out = reinterpret_cast<qint16*>(data);

    for (int i = 0; i < numSamples; ++i) {
        double freq = toneMode_ ? currentFreq_ : f0List_.value(currentFrame_, 0.0);
        double sample = generateSample(freq);
        qint16 s = static_cast<qint16>(sample * 32767.0);

        for (int i = 0; i < numSamples; ++i) {
            double time = static_cast<double>(sampleIndex_) / format_.sampleRate();
            double freq = toneMode_ ? currentFreq_ : interpolatedFrequency(time);

            double sample = generateSample(freq);
            qint16 s = static_cast<qint16>(sample * 32767.0);

            for (int ch = 0; ch < format_.channelCount(); ++ch)
                *out++ = s;

            ++sampleIndex_;

            // cursorMoved シグナルはフレーム境界で送信
            if (!toneMode_) {
                int newFrame = static_cast<int>((sampleIndex_ * fps_) / format_.sampleRate());
                if (newFrame != currentFrame_) {
                    currentFrame_ = newFrame;
                    emit cursorMoved(currentFrame_);
                    if (currentFrame_ >= f0List_.size()) {
                        stopped_ = true;
                        break;
                    }
                }
            }
        }

        sampleIndex_++;

        if (!toneMode_) {
            int newFrame = static_cast<int>((sampleIndex_ * fps_) / sampleRate);
            if (newFrame != currentFrame_) {
                currentFrame_ = newFrame;
                emit cursorMoved(currentFrame_);
                if (currentFrame_ >= f0List_.size()) {
                    stopped_ = true;
                    break;
                }
            }
        }
    }

    return reinterpret_cast<char*>(out) - data;
}

double WaveGenerator::generateSample(double freq)
{
    if (freq <= 0.0 || format_.sampleRate() <= 0)
        return 0.0;

    double sampleRate = format_.sampleRate();

    // 現在の位相から sin 波生成（滑らか矩形波でもいい）
    double sample = tanh(sin(2.0 * M_PI * phase_) * 5.0); // 丸めた矩形波

    // 位相を進める
    phase_ += freq / sampleRate;
    if (phase_ >= 1.0) phase_ -= 1.0;

    return sample;
}

double WaveGenerator::interpolatedFrequency(double timeInSec) {
    if (fps_ <= 0.0 || f0List_.isEmpty())
        return 0.0;

    double framePos = timeInSec * fps_; // 秒 → フレーム位置（実数）
    int frame0 = static_cast<int>(framePos);
    int frame1 = frame0 + 1;

    if (frame0 < 0) frame0 = 0;
    if (frame1 >= f0List_.size()) frame1 = f0List_.size() - 1;

    double f0 = f0List_.value(frame0, 0.0);
    double f1 = f0List_.value(frame1, 0.0);
    double alpha = framePos - frame0;

    return (1.0 - alpha) * f0 + alpha * f1; // 線形補間
}

