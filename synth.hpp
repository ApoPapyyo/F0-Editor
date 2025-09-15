#ifndef SYNTH_H
#define SYNTH_H

#include <QVector>
#include <QIODevice>
#include <QAudioSink>
#include <QByteArray>

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
        qsizetype* cur;
        double freq, phase;
    };
    const QAudioFormat m_format;
    qsizetype m_cur_dummy;
    Status m_status;
public:
    Generator(QAudioFormat format, QObject *parent = nullptr);
    ~Generator();
    void setFreq(double f);
    void setData(const QByteArray& data);
    void setCurVar(qsizetype* v);
    qsizetype getCur() const;
    bool isPlaying() const;
    qint64 readData(char *data, qint64 maxlen) override;
    int getSampleRate() const;
    int getChannelCount() const;
    QAudioFormat getFormat() const;

protected:
    qint64 writeData(const char*, qint64) override;
};

class Synth {
    QAudioSink* m_out;
    Generator* m_gen;
    QVector<double> m_freq_cache;
    QByteArray m_sample_cache;
public:
    Synth();
    ~Synth();
    void setData(const QVector<double>& freqs, double time_per_frame);
    void setFreq(double f);
    void setCurVar(qsizetype* v = nullptr);
    void play();
    void stop();
    bool isPlaying() const;
    int getSampleRate() const;
    int getChannelCount() const;
    int getFormatSize() const;

};

#endif // SYNTH_H
