#include "synth.h"
#include "synthworker.h"

Synth::Synth(QObject* parent) : QObject(parent) {
    m_worker = new SynthWorker();
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &SynthWorker::cursorMoved, this, &Synth::cursorMoved);
    m_workerThread.start();
}

Synth::~Synth() {
    m_worker->stop();
    m_workerThread.quit();
    m_workerThread.wait();
}

void Synth::setF0(const QList<double>& f0List, double fps) {
    QMetaObject::invokeMethod(m_worker, [this, f0List, fps]() {
        m_worker->setupF0(f0List, fps);
    });
}

void Synth::play() {
    QMetaObject::invokeMethod(m_worker, &SynthWorker::play);
}

void Synth::stop() {
    QMetaObject::invokeMethod(m_worker, &SynthWorker::stop);
}

void Synth::playTone(double freq) {
    QMetaObject::invokeMethod(m_worker, [this, freq]() {
        m_worker->playTone(freq);
    });
}

void Synth::setFrequency(double freq) {
    QMetaObject::invokeMethod(m_worker, [this, freq]() {
        m_worker->setFrequency(freq);
    });
}
