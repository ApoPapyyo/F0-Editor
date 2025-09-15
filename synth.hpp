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
    const int m_sr;
    const int m_ch;
    qsizetype m_cur_dummy;
    Status m_status;
public:
    Generator(int sample_rate = 44100, int channel_count = 2, QObject *parent = nullptr);
    ~Generator();
    void setFreq(double f);
    void setData(const QByteArray& data);
    void setCurVar(qsizetype* v);
    qsizetype getCur() const;
    bool isPlaying() const;
    qint64 readData(char *data, qint64 maxlen) override;
    int getSampleRate() const;
    int getChannelCount() const;

protected:
    qint64 writeData(const char*, qint64) override;
};

class Synth {
    QAudioSink* m_out;
    Generator* m_gen;
    QVector<double> m_freq_cache;
    QByteArray m_sample_cache;
public:
    Synth(int sr);
    ~Synth();
    void setData(const QVector<double>& freqs, double time_per_frame);
    void setFreq(double f);
    void setCurVar(qsizetype* v = nullptr);
    void play();
    void stop();
    bool isPlaying() const;

};

#endif // SYNTH_H
