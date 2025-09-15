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
QByteArray generateContinuousSine(double time_per_frame, const QVector<double>& f0_data,
                                  int sample_rate = 44100, int channel_count = 2)
{
    const qint64 sample_size = sizeof(qint16) * channel_count;
    const qint64 frame_bytes = static_cast<qint64>(time_per_frame * sample_rate * sample_size);
    const qint64 total_bytes = frame_bytes * f0_data.size();

    QByteArray audio(total_bytes, 0);

    Generator gen(sample_rate, channel_count);

    for (int i = 0; i < f0_data.size(); ++i) {
        gen.setFreq(f0_data[i]);
        qint64 offset = i * frame_bytes;
        qint64 written = gen.readData(audio.data() + offset, frame_bytes);
    }

    return audio;
}
}


Generator::Generator(int sr, int ch, QObject* parent)
    : QIODevice(parent)
    , m_sr(sr)
    , m_ch(ch)
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
    qint16 *_buf = reinterpret_cast<qint16*>(data);
    const qsizetype sample_size = sizeof(qint16) * m_ch;
    const qsizetype sample_count = maxlen / sample_size;

    //m_status.cur: 全チャンネル含めたサンプル単位
    //なのでsample_sizeをかけるとバイト数になる

    auto buf = [=](qsizetype i, int ch) -> qint16& {
        if(ch >= m_ch) ch = 0;
        return _buf[i * m_ch + ch];
    };

    qsizetype size = sample_count * sample_size;

    if(m_status.type == eContentType::Realtime && m_sr > 0) {
        for(int i = 0; i < sample_count; i++) {
            double t = (double)i / (double)m_sr;
            double instantaneous_phase = 2.0 * M_PI * m_status.freq * t + m_status.phase;
            buf(i, 0) = static_cast<qint16>((0.6 * std::tanh(1.0 * std::sin(instantaneous_phase))) * 32767.0);
            for(int j = 0; j < m_ch-1; j++) buf(i, j+1) = buf(i, j);
            if (i == sample_count - 1) {
                m_status.phase = instantaneous_phase + 2.0 * M_PI * m_status.freq / (double)m_sr;
            }
        }
    } else if(m_status.type == eContentType::Data && m_status.data) {
        const qsizetype data_size = m_status.data->size() / sample_size;
        if(data_size > *m_status.cur && *m_status.cur >= 0) {
            if(size > (data_size - *m_status.cur) * sample_size) {
                size = (data_size - *m_status.cur) * sample_size;
            }
            memcpy(data, &m_status.data->data()[*m_status.cur * sample_size], size);
            if(sample_count >= (data_size - *m_status.cur)) {
                m_status.type = eContentType::None;
                //*m_status.cur = 0;
                m_status.data = nullptr;
                m_status.freq = 0.0;
                m_status.phase = 0.0;
            }
            *m_status.cur += size/sample_size;
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

int Generator::getSampleRate() const
{
    return m_sr;
}

int Generator::getChannelCount() const
{
    return m_ch;
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
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    // デフォルト出力デバイスを取得
    QAudioDevice dev = QMediaDevices::defaultAudioOutput();
    // 出力デバイス作成
    m_out = new QAudioSink(dev, format);

    m_gen = new Generator();
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
        m_sample_cache = generateContinuousSine(time_per_frame, m_freq_cache, m_gen->getSampleRate(), m_gen->getChannelCount());
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
