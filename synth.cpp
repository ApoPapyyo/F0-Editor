#include "synth.h"
#include <QDebug>

Synth::Synth(QObject* parent)
    : QObject(parent)
    , m_audioSink(nullptr)
    , m_generator(nullptr)
    , m_audioThread(nullptr)
    , m_fps(60.0)
    , m_currentFrame(0)
{
    initializeAudio();
}

Synth::~Synth()
{
    cleanup();
}

void Synth::initializeAudio()
{
    // オーディオフォーマットの設定
    m_format.setSampleRate(SAMPLE_RATE);
    m_format.setChannelCount(1); // モノラル
    m_format.setSampleFormat(QAudioFormat::Float);
    
    // デフォルトオーディオデバイスを取得
    QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
    
    // オーディオシンクの作成
    m_audioSink = new QAudioSink(audioDevice, m_format, this);
    m_audioSink->setBufferSize(BUFFER_SIZE * sizeof(float));
    
    // オーディオジェネレーターを別スレッドで作成
    m_audioThread = new QThread(this);
    m_generator = new AudioGenerator();
    m_generator->moveToThread(m_audioThread);
    
    // シグナル接続
    connect(m_generator, &AudioGenerator::frameChanged, 
            this, &Synth::onFrameChanged, Qt::QueuedConnection);
    
    // スレッド開始
    m_audioThread->start();
    
    // ジェネレーターを開く
    m_generator->open(QIODevice::ReadOnly);
}

void Synth::cleanup()
{
    if (m_audioSink) {
        m_audioSink->stop();
    }
    
    if (m_generator) {
        m_generator->stopPlayback();
        m_generator->close();
    }
    
    if (m_audioThread) {
        m_audioThread->quit();
        m_audioThread->wait();
        delete m_generator;
        m_generator = nullptr;
    }
}

void Synth::setF0(const QList<double>& freqs, double fps)
{
    m_f0Data = freqs;
    m_fps = fps;
    
    if (m_generator) {
        m_generator->setF0Data(freqs, fps);
    }
}

void Synth::play(int frame_offset)
{
    if (!m_generator || m_f0Data.isEmpty()) {
        return;
    }
    
    stop(); // 既存の再生を停止
    
    m_currentFrame = frame_offset;
    m_generator->startPlayback(frame_offset);
    m_audioSink->start(m_generator);
}

void Synth::stop()
{
    if (m_audioSink) {
        m_audioSink->stop();
    }
    
    if (m_generator) {
        m_generator->stopPlayback();
    }
}

void Synth::playTone(double freq)
{
    if (!m_generator) {
        return;
    }
    
    stop(); // 既存の再生を停止
    
    m_generator->startTone(freq);
    m_audioSink->start(m_generator);
}

void Synth::setFrequency(double freq)
{
    if (m_generator) {
        m_generator->setCurrentFreq(freq);
    }
}

void Synth::onFrameChanged(int frame)
{
    m_currentFrame = frame;
    emit cursorMoved(frame);
}

// AudioGenerator実装

AudioGenerator::AudioGenerator(QObject* parent)
    : QIODevice(parent)
    , m_fps(60.0)
    , m_currentFrame(0)
    , m_frameOffset(0)
    , m_isPlaying(false)
    , m_isTonePlaying(false)
    , m_currentFreq(440.0)
    , m_toneFreq(440.0)
    , m_phase(0.0)
    , m_sampleIndex(0)
    , m_samplesPerFrame(SAMPLE_RATE / 60) // デフォルト60fps
{
}

void AudioGenerator::setF0Data(const QList<double>& data, double fps)
{
    QMutexLocker locker(&m_mutex);
    m_f0Data = data;
    m_fps = fps;
    m_samplesPerFrame = static_cast<int>(SAMPLE_RATE / fps);
}

void AudioGenerator::startPlayback(int frameOffset)
{
    QMutexLocker locker(&m_mutex);
    m_frameOffset = frameOffset;
    m_currentFrame = frameOffset;
    m_sampleIndex = 0;
    m_isPlaying = true;
    m_isTonePlaying = false;
}

void AudioGenerator::startTone(double freq)
{
    QMutexLocker locker(&m_mutex);
    m_toneFreq = freq;
    m_currentFreq = freq;
    m_isTonePlaying = true;
    m_isPlaying = false;
}

void AudioGenerator::stopPlayback()
{
    QMutexLocker locker(&m_mutex);
    m_isPlaying = false;
    m_isTonePlaying = false;
}

void AudioGenerator::setCurrentFreq(double freq)
{
    QMutexLocker locker(&m_mutex);
    if (m_isTonePlaying) {
        m_toneFreq = freq;
        m_currentFreq = freq;
    }
}

qint64 AudioGenerator::readData(char* data, qint64 maxlen)
{
    qDebug() << "readData called, len =" << maxlen;
    QMutexLocker locker(&m_mutex);
    
    if (!m_isPlaying && !m_isTonePlaying) {
        // 無音を出力
        memset(data, 0, maxlen);
        return maxlen;
    }
    
    const int sampleCount = maxlen / sizeof(float);
    float* buffer = reinterpret_cast<float*>(data);
    
    generateSamples(buffer, sampleCount);
    
    return maxlen;
}

qint64 AudioGenerator::writeData(const char* data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    return -1; // 読み取り専用
}

void AudioGenerator::generateSamples(float* buffer, int sampleCount)
{
    for (int i = 0; i < sampleCount; ++i) {
        double freq = 0.0;
        
        if (m_isTonePlaying) {
            freq = m_toneFreq;
        } else if (m_isPlaying) {
            // フレームが変わったかチェック
            int newFrame = m_frameOffset + (m_sampleIndex / m_samplesPerFrame);
            
            if (newFrame != m_currentFrame && newFrame < m_f0Data.size()) {
                m_currentFrame = newFrame;
                emit frameChanged(m_currentFrame);
            }
            
            // 現在のフレームの周波数を取得
            if (m_currentFrame < m_f0Data.size()) {
                freq = m_f0Data[m_currentFrame];
            } else {
                // データの終端に達したら停止
                m_isPlaying = false;
                buffer[i] = 0.0f;
                continue;
            }
        }
        
        // サンプル生成
        buffer[i] = static_cast<float>(generateSample(freq));
        m_sampleIndex++;
    }
}

double AudioGenerator::generateSample(double freq)
{
    if (freq <= 0.0) {
        return 0.0;
    }
    
    // 丸めの矩形波生成 (sin を tanh に通す)
    double sineWave = sin(m_phase);
    double roundedSquare = tanh(sineWave * 5.0); // 5.0は丸めの強度
    
    // 位相を進める（周波数が変わっても位相は連続）
    double phaseIncrement = 2.0 * PI * freq / SAMPLE_RATE;
    m_phase += phaseIncrement;
    
    // 位相オーバーフロー対策
    if (m_phase >= 2.0 * PI) {
        m_phase -= 2.0 * PI;
    }
    
    return roundedSquare * 0.3; // 音量調整
}
