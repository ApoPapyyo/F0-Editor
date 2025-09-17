#ifndef SYNTH_H
#define SYNTH_H

#include <QVector>
#include <QIODevice>
#include <QAudioSink>
#include <QByteArray>
#include <QTimer>

class Generator : public QIODevice {
    Q_OBJECT
private:
    enum class eContentType {
        None,
        Realtime,
        Data
    };
    struct Status {
        eContentType type;
        const QByteArray* data;
        qint64* cur;
        double freq, phase, timeout;
    };
    const QAudioFormat m_format;
    qint64 m_cur_dummy;
    Status m_status;
public:
    Generator(QAudioFormat format, QObject *parent = nullptr);
    ~Generator();
    void setFreq(double f, double timeout = 0.0);
    void setData(const QByteArray& data);
    void setCurVar(qint64* v);
    qint64 getCur() const;
    bool isPlaying() const;
    int getSampleRate() const;
    int getChannelCount() const;
    QAudioFormat getFormat() const;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64) override;
};

class Synth : QObject {
    QTimer* m_timer;
    QAudioSink* m_sink;
    Generator* m_gen;
    QVector<double> m_freq_cache;
    QByteArray m_sample_cache;
public:
    Synth(QObject* parent = nullptr);
    ~Synth();
    void setData(const QVector<double>& freqs, double time_per_frame);
    void setFreq(double f, double timeout = 0.0);
    void setCurVar(qint64* v = nullptr);
    void play();
    void stop();
    bool isPlaying() const;
    int getSampleRate() const;
    int getChannelCount() const;
    int getFormatSize() const;

};

#endif // SYNTH_H
