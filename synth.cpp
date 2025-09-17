#include "synth.hpp"

#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include <cmath>


#define Warning(mes) qDebug() << QObject::tr("警告") << ": " << mes
#define Error(mes) qDebug() << QObject::tr("エラー") << ": " << mes


namespace {
QByteArray encodeData(double data, QAudioFormat::SampleFormat format)
{
    QByteArray ret;
    if(data < -1.0 || data > 1.0) return ret;

    switch(format) {
    case QAudioFormat::SampleFormat::Float:{
        float d = data;
        ret.resize(sizeof(float));
        memcpy(ret.data(), &d, sizeof(float));
        break;
    }
    case QAudioFormat::SampleFormat::Int16:{
        qint16 d = static_cast<qint16>(data * 32767.0);
        ret.resize(sizeof(qint16));
        memcpy(ret.data(), &d, sizeof(qint16));
        break;
    }
    case QAudioFormat::SampleFormat::Int32:{
        qint32 d = static_cast<qint32>(data * 2147483647.0);
        ret.resize(sizeof(qint32));
        memcpy(ret.data(), &d, sizeof(qint32));
        break;
    }
    case QAudioFormat::SampleFormat::UInt8:{
        quint8 d = static_cast<quint8>(data * 128.0 + 128.0 > 255.0 ? 255.0 : data * 128.0 + 128.0);
        ret.resize(sizeof(quint8));
        memcpy(ret.data(), &d, sizeof(quint8));
        break;
    }
    default:
        break;
    }

    return ret;
}

double decodeData(const QByteArray& data, QAudioFormat::SampleFormat format)
{
    switch(format) {
    case QAudioFormat::SampleFormat::Float:{
        if(data.size() < sizeof(float)) break;
        float d;
        memcpy(&d, data.data(), sizeof(float));
        return d;
    }
    case QAudioFormat::SampleFormat::Int16:{
        if(data.size() < sizeof(qint16)) break;
        qint16 d;
        memcpy(&d, data.data(), sizeof(qint16));
        return (double)d / 32767.0;
    }
    case QAudioFormat::SampleFormat::Int32:{
        if(data.size() < sizeof(qint32)) break;
        qint32 d;
        memcpy(&d, data.data(), sizeof(qint32));
        return (double)d / 2147483647.0;
    }
    case QAudioFormat::SampleFormat::UInt8:{
        if(data.size() < sizeof(quint8)) break;
        quint8 d;
        memcpy(&d, data.data(), sizeof(quint8));
        return ((double)d - 128.0) / 128.0;
    }
    default:
        break;
    }
    return 0.0;
}

qint64 sizeofFmt(QAudioFormat::SampleFormat format)
{
    switch(format) {
    case QAudioFormat::SampleFormat::Float: return sizeof(float);
    case QAudioFormat::SampleFormat::Int16: return sizeof(qint16);
    case QAudioFormat::SampleFormat::Int32: return sizeof(qint32);
    case QAudioFormat::SampleFormat::UInt8: return sizeof(quint8);
    default: return 0;
    }
}

QByteArray generateContinuousSine(double time_per_frame, const QVector<double>& f0_data,
                                  QAudioFormat fmt)
{
    const qint64 fmt_size = sizeofFmt(fmt.sampleFormat());
    const qint64 sample_rate = fmt.sampleRate();
    const qint64 channel_count = fmt.channelCount();
    const qint64 sample_size = fmt_size * channel_count;
    const qint64 frame_bytes = static_cast<qint64>(time_per_frame * sample_rate * sample_size);
    const qint64 total_bytes = frame_bytes * f0_data.size();

    QByteArray audio(total_bytes, 0);

    Generator gen(fmt);
    for (int i = 0; i < f0_data.size(); ++i) {
        gen.setFreq(f0_data[i], 0.01);
        qint64 offset = i * frame_bytes;
        auto b = gen.read(frame_bytes);
        if(b.size() == 0) continue;
        else if(b.size() > frame_bytes) {
            Error(QObject::tr("波形生成に失敗しました。"));
            return QByteArray{};
        }
        memcpy(audio.data() + offset, b.data(), b.size());
    }

    return audio;
}
}

Generator::Generator(QAudioFormat format, QObject* parent)
    : QIODevice(parent)
    , m_format(format)
    , m_status{eContentType::None, nullptr, &m_cur_dummy, 0.0, 0.0, 0.0}
{

    if(!m_format.isValid() || sizeofFmt(m_format.sampleFormat()) == 0) {
        Error(tr("対応していないフォーマットです。:") << m_format);
        return;
    } else {
        qDebug() << "Opening " << m_format;
        open(QIODevice::ReadOnly);
        qDebug() << "Open " << (isOpen() ? "success." : "failed.");
    }
}

Generator::~Generator()
{
    if(isOpen()) close();
}

qint64 Generator::readData(char* data, qint64 maxlen)
{
    if(!m_format.isValid()) return -1;
    if(sizeofFmt(m_format.sampleFormat()) <= 0) return -1;
    const auto ch = m_format.channelCount();
    const auto sr = m_format.sampleRate();
    const auto fmt = m_format.sampleFormat();
    const auto fmt_s = sizeofFmt(fmt);
    const qint64 sample_size = fmt_s * ch;
    const qint64 sample_timeout = static_cast<qint64>(m_status.timeout * sr);
    const qint64 sample_count = (sample_timeout == 0 ? maxlen/sample_size : (sample_timeout <= maxlen/sample_size ? sample_timeout : maxlen/sample_size));

    qint64 size = sample_count * sample_size;
    if(m_status.type == eContentType::Realtime && sr > 0) {
        for(int i = 0; i < sample_count; i++) {
            double t = (double)i / (double)sr;
            double instantaneous_phase = 2.0 * M_PI * m_status.freq * t + m_status.phase;
            double d = (0.6 * std::tanh(1.0 * std::sin(instantaneous_phase)));
            auto e = encodeData(d, fmt);
            for(int j = 0; j < ch; j++) {
                const auto&& offset = (ch*i + j) * fmt_s;
                memcpy(data + offset, e.data(), fmt_s);
            }
            if (i == sample_count - 1) {
                m_status.phase = instantaneous_phase + 2.0 * M_PI * m_status.freq / (double)sr;
            }
        }
    } else if(m_status.type == eContentType::Data && m_status.data) {
        const qint64 data_size = m_status.data->size() / sample_size;
        if(data_size > *m_status.cur && *m_status.cur >= 0) {
            if(size > (data_size - *m_status.cur) * sample_size) {
                size = (data_size - *m_status.cur) * sample_size;
            }
            memcpy(data, &m_status.data->data()[*m_status.cur * sample_size], size);
            if(sample_count >= (data_size - *m_status.cur)) {
                m_status.type = eContentType::None;
                //*m_status.cur = 0;
                m_status.freq = 0.0;
                m_status.phase = 0.0;
                m_status.timeout = 0.0;
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

void Generator::setFreq(double f, double timeout)
{
    if(f <= 0.0) {//停止
        m_status.type = eContentType::None;
        //*m_status.cur = 0;
        m_status.freq = 0.0;
        m_status.phase = 0.0;
        m_status.timeout = 0.0;
    } else {
        if(m_status.type != eContentType::Realtime) {
            //*m_status.cur = 0;
            m_status.phase = 0.0;
            m_status.type = eContentType::Realtime;
        }
        m_status.freq = f;
        m_status.timeout = timeout >= 0 ? timeout : 0.0;
    }
}

void Generator::setData(const QByteArray& data)
{

    m_status.type = eContentType::Data;
    m_status.data = &data;
}

qint64 Generator::getCur() const
{
    return m_status.type == eContentType::Data ? *m_status.cur : 0;
}

int Generator::getSampleRate() const
{
    return m_format.sampleRate();
}

int Generator::getChannelCount() const
{
    return m_format.channelCount();
}

QAudioFormat Generator::getFormat() const
{
    return m_format;
}

bool Generator::isPlaying() const
{
    return m_status.type != eContentType::None;
}

void Generator::setCurVar(qint64* v)
{
    if(!v) {
        m_status.cur = &m_cur_dummy;
    } else {
        m_status.cur = v;
    }
}

Synth::Synth(QObject* parent)
    : QObject(parent)
    , m_timer(nullptr)
    , m_sink(nullptr)
    , m_gen(nullptr)
{
    QAudioDevice dev = QMediaDevices::defaultAudioOutput();
    QAudioFormat format = dev.preferredFormat();
    m_sink = new QAudioSink(dev, format, this);
    m_gen = new Generator(format, this);
#ifdef Q_OS_MAC
    m_timer = new QTimer(this);
    auto push = m_sink->start();
    connect(m_timer, &QTimer::timeout, this, [=]() {
        qint64 period = m_sink->periodSize();
        if (period <= 0)
            period = 4096; // fallback

        if (m_sink->bytesFree() >= period) {
            QByteArray buf = m_gen->read(period);
            if (!buf.isEmpty())
                push->write(buf);
        }
    });
#else
#endif
}

Synth::~Synth()
{
    stop();
    if(m_timer) m_timer->stop();
    if(m_sink) delete m_sink;
    if(m_gen) delete m_gen;
    if(m_timer) delete m_timer;
}

void Synth::setData(const QVector<double>& freqs, double time_per_frame)
{
    if(freqs != m_freq_cache) {
        m_freq_cache = freqs;
        m_sample_cache = generateContinuousSine(time_per_frame, m_freq_cache, m_gen->getFormat());
    }
    m_gen->setData(m_sample_cache);
}

void Synth::setFreq(double f, double timeout)
{
    m_gen->setFreq(f, timeout);
}

void Synth::setCurVar(qint64* cur)
{
    m_gen->setCurVar(cur);
}

void Synth::play()
{
#ifdef Q_OS_MAC
    m_timer->start(5);
#else
    m_sink->start(m_gen);
#endif
}

void Synth::stop()
{
#ifdef Q_OS_MAC
    m_timer->stop();
#else
    m_gen->setFreq(0.0);
    m_sink->stop();
#endif
}

bool Synth::isPlaying() const
{
    return m_gen->isPlaying();
}

int Synth::getSampleRate() const
{
    return m_gen->getSampleRate();
}

int Synth::getChannelCount() const
{
    return m_gen->getChannelCount();
}

int Synth::getFormatSize() const
{
    return sizeofFmt(m_gen->getFormat().sampleFormat());
}
