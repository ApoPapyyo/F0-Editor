#include "synthworker.h"
#include <QAudioDevice>
#include <QAudioOutput>
#include <QtMath>

#include <QMediaDevices>
#include <QTimer>
/*
SynthWorker::SynthWorker(QObject* parent) : QObject(parent) {
    initAudio();
}
*/

SynthWorker::~SynthWorker() {
    stop();
    if(m_audioSink) delete m_audioSink;
    if(m_audioDevice) delete m_audioDevice;
}

// synthworker.cpp の主要部分のみ抜粋・修正例


SynthWorker::SynthWorker(QObject* parent) : QObject(parent) {
    // preferredFormat を使う
    auto device = QMediaDevices::defaultAudioOutput();
    m_format = device.preferredFormat();
    m_format.setChannelCount(1);
    m_format.setSampleRate(48000); // 必要に応じて preferredFormat を使い切るのもOK

    m_audioSink = new QAudioSink(device, m_format, this);
    m_audioDevice = m_audioSink->start();

    connect(m_audioDevice, &QIODevice::readyRead, this, [this]() {
        // QIODevice の readyRead は読み込み時なので、
        // 書き込み処理には使わないほうがいい
    });

    // バッファを継続的に書き込むため、QTimerで繰り返す
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        qint64 freeBytes = m_audioSink->bytesFree();
        if (freeBytes > 0) {
            generateNextBuffer(freeBytes);
        }
    });
    timer->start(10); // 10ms間隔でバッファ供給
}

void SynthWorker::generateNextBuffer(qint64 bytes) {
    if (m_stopRequested.load()) return;
    if (!m_audioDevice || !m_audioDevice->isOpen()) return;

    // 16bit mono サンプル数計算
    int samples = bytes / sizeof(qint16);
    QByteArray buffer;
    buffer.resize(samples * sizeof(qint16));
    qint16* data = reinterpret_cast<qint16*>(buffer.data());

    const double dt = 1.0 / m_format.sampleRate();

    for (int i = 0; i < samples; ++i) {
        double freq = m_playingF0 && m_currentFrame < m_f0List.size()
        ? m_f0List[m_currentFrame]
        : m_currentFreq;

        // 丸めた矩形波 (位相0~1)
        double val = (m_phase < 0.5) ? 0.7 : -0.7;
        data[i] = static_cast<qint16>(val * 32767);

        m_phase += freq * dt;
        if (m_phase >= 1.0) m_phase -= 1.0;

        // フレーム更新処理（fpsに応じて）
        if (m_playingF0) {
            double timeSec = (m_currentFrame / m_fps);
            int newFrame = static_cast<int>((i * dt) * m_fps);
            if (newFrame != m_currentFrame && newFrame < m_f0List.size()) {
                m_currentFrame = newFrame;
                emit cursorMoved(m_currentFrame);
            }
        }
        m_currentFreq = freq;
    }
    m_audioDevice->write(buffer);
}


void SynthWorker::initAudio() {
    m_format.setSampleRate(static_cast<int>(m_sampleRate));
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    m_audioSink = new QAudioSink(m_format, this);
    m_audioDevice = m_audioSink->start();
    connect(m_audioDevice, &QIODevice::readyRead, [this]() {
        generateNextBuffer(m_audioSink->bytesFree());
    });
}

void SynthWorker::setupF0(const QList<double>& f0List, double fps) {
    QMutexLocker locker(&m_mutex);
    m_f0List = f0List;
    m_fps = fps;
    m_currentFrame = 0;
    m_playingF0 = true;
}

void SynthWorker::play() {
    QMutexLocker locker(&m_mutex);
    m_playingF0 = true;
    m_currentFrame = 0;
}

void SynthWorker::playTone(double freq) {
    QMutexLocker locker(&m_mutex);
    m_playingF0 = false;
    m_currentFreq = freq;
}

void SynthWorker::setFrequency(double freq) {
    QMutexLocker locker(&m_mutex);
    m_currentFreq = freq;
}

void SynthWorker::stop() {
    m_stopRequested = true;

    if (m_audioSink) {
        m_audioSink->stop();
    }

    // m_audioDevice は start() で作られているため
    // 再利用はできないので破棄してnullptrにする
    if (m_audioDevice) {
        m_audioDevice->close();
        m_audioDevice = nullptr;
    }

    m_stopRequested = false;
}

/*
void SynthWorker::generateNextBuffer(qint64 bytes) {
    if (m_stopRequested) return;

    QByteArray buffer;
    buffer.resize(bytes);
    qint16* data = reinterpret_cast<qint16*>(buffer.data());
    int samples = bytes / sizeof(qint16);

    const double dt = 1.0 / m_sampleRate;
    const double frameDur = 1.0 / m_fps;
    double nextFreq = m_currentFreq;

    for (int i = 0; i < samples; ++i) {
        if (m_playingF0 && m_currentFrame < m_f0List.size()) {
            nextFreq = m_f0List[m_currentFrame];
        }

        // 波形生成（丸めた矩形波）
        double value = qSin(2.0 * M_PI * m_phase);
        value = value >= 0 ? 0.7 : -0.7;

        data[i] = static_cast<qint16>(value * 32767);
        m_phase += nextFreq * dt;
        if (m_phase >= 1.0) m_phase -= 1.0;

        // フレーム更新チェック
        int newFrame = static_cast<int>((i / m_sampleRate) * m_fps);
        if (m_playingF0 && newFrame != m_currentFrame && newFrame < m_f0List.size()) {
            m_currentFrame = newFrame;
            emit cursorMoved(m_currentFrame);
        }

        m_currentFreq = nextFreq;
    }

    m_audioDevice->write(buffer);
}
*/
