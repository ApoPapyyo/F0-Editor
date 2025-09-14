#include "synth.hpp"

#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QObject>
#include <iostream>
#include <cmath>


#define Warning(mes) std::cerr << QObject::tr("警告").toStdString() << ": " << mes << std::endl
#define Error(mes) std::cerr << QObject::tr("エラー").toStdString() << ": " << mes << std::endl

namespace {
QByteArray generateContinuousSine(double timePerFrame, const QVector<double>& f0Data, int sampleRate = 44100)
{
    QByteArray audio;

    // 全体サンプル数 ≒ 最終時刻 * sampleRate
    int totalSamples = int(timePerFrame * f0Data.size() * sampleRate);
    audio.resize(totalSamples * sizeof(int16_t));

    int16_t* ad = reinterpret_cast<int16_t*>(audio.data());

    double phase = 0.0;
    qsizetype idx = 0;

    for (int j = 0; j < f0Data.size(); ++j) {
        double freq = (f0Data[j] <= 0.0 || std::isnan(f0Data[j])) ? 0.0 : f0Data[j];

        int samplesThisSeg = int(timePerFrame * sampleRate);
        for (int n = 0; n < samplesThisSeg; ++n) {
            double t = double(n) / sampleRate;
            double sample = 0.0;
            if (freq > 0.0) {
                double instantaneous_phase = 2.0 * M_PI * freq * t + phase;
                sample = 0.6 * std::tanh(5.0 * std::sin(instantaneous_phase));
                if (n == samplesThisSeg - 1) {
                    phase = instantaneous_phase + 2.0 * M_PI * freq * (1.0 / sampleRate);
                }
            } else {
                phase = 0.0;
            }
            if (idx < audio.size()) {
                ad[idx++] = static_cast<int16_t>(sample * 32767.0);
            }
        }
    }

    return audio;
}
}


Generator::Generator(QObject* parent)
    : QIODevice(parent)
    , m_sr(0)
    , m_status{eContentType::None, nullptr, &m_cur_dummy, 0.0, 0.0}
{
    open(QIODevice::ReadOnly);
}

Generator::~Generator()
{
    close();
}

qint64 Generator::readData(char* data, qint64 maxlen)
{
    qint16 *buf = reinterpret_cast<qint16*>(data);
    int samplec = maxlen / sizeof(qint16);

    qsizetype size = maxlen;

    if(m_status.type == eContentType::Realtime && m_sr > 0) {
        for(int i = 0; i < samplec; i++) {
            double t = (double)i / (double)m_sr;
            double instantaneous_phase = 2.0 * M_PI * m_status.freq * t + m_status.phase;
            buf[i] = static_cast<qint16>((0.6 * std::tanh(5.0 * std::sin(instantaneous_phase))) * 32767.0);
            if (i == samplec - 1) {
                m_status.phase = instantaneous_phase + 2.0 * M_PI * m_status.freq / (double)m_sr;
            }

        }
    } else if(m_status.type == eContentType::Data && m_status.data) {
        if(m_status.data->size() > *m_status.cur && *m_status.cur >= 0) {
            if(maxlen < (m_status.data->size() - *m_status.cur)) {
                size = maxlen;
            } else {//最後の書き込み
                size = m_status.data->size() - *m_status.cur;
                m_status.type = eContentType::None;
                //*m_status.cur = 0;
                m_status.data = nullptr;
                m_status.freq = 0.0;
                m_status.phase = 0.0;
            }
            memcpy(data, &m_status.data->data()[*m_status.cur], size);
            *m_status.cur += size;
        } else {
            size = 0;
        }
    } else {
        size = 0;
    }

    return size;
}

qint64 Generator::writeData(const char*, qint64)
{
    return -1;
}

void Generator::setSampleRate(int sr)
{
    if(sr > 0) m_sr = sr;
}

void Generator::setFreq(double f)
{
    if(f <= 0.0) {//停止
        m_status.type = eContentType::None;
        //*m_status.cur = 0;
        m_status.data = nullptr;
        m_status.freq = 0.0;
        m_status.phase = 0.0;
    } else {
        if(m_status.type != eContentType::Realtime) {
            //*m_status.cur = 0;
            m_status.data = nullptr;
            m_status.phase = 0.0;
            m_status.type = eContentType::Realtime;
        }
        m_status.freq = f;
    }
}

void Generator::setData(const QByteArray& data)
{
    if(data.size() == 0) {//停止
        m_status.type = eContentType::None;
        //*m_status.cur = 0;
        m_status.data = nullptr;
        m_status.freq = 0.0;
        m_status.phase = 0.0;
    } else {
        m_status.type = eContentType::Data;
        m_status.data = &data;
    }
}

qsizetype Generator::getCur() const
{
    return m_status.type == eContentType::Data ? *m_status.cur : 0;
}

bool Generator::isPlaying() const
{
    return m_status.type != eContentType::None;
}

void Generator::setCurVar(qsizetype* v)
{
    if(!v) {
        m_status.cur = &m_cur_dummy;
    } else {
        m_status.cur = v;
    }
}

Synth::Synth(int sr)
    : m_out(nullptr)
    , m_gen(nullptr)
{
    QAudioFormat format;
    format.setSampleRate(sr);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    // デフォルト出力デバイスを取得
    QAudioDevice dev = QMediaDevices::defaultAudioOutput();
    // 出力デバイス作成
    m_out = new QAudioSink(dev, format);

    m_gen = new Generator();

    m_gen->setSampleRate(sr);
}

Synth::~Synth()
{
    if(m_out) {
        delete m_out;
    }
    if(m_gen) {
        delete m_gen;
    }
}

void Synth::setData(const QVector<double>& freqs, double time_per_frame)
{
    if(freqs != m_freq_cache) {
        m_freq_cache = freqs;
        m_sample_cache = generateContinuousSine(time_per_frame, m_freq_cache);
    }
    m_gen->setData(m_sample_cache);
}

void Synth::setFreq(double f)
{
    m_gen->setFreq(f);
}

void Synth::setCurVar(qsizetype* cur)
{
    m_gen->setCurVar(cur);
}

void Synth::play()
{
    m_out->start(m_gen);
}

void Synth::stop()
{
    m_gen->setFreq(0.0);
    m_out->stop();
}

bool Synth::isPlaying() const
{
    return m_gen->isPlaying();
}
