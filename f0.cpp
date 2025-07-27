#include "f0.h"
#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include <QMap>
F0::F0()
    : _path()
    , _data()
    , _fps(0)
    , _changed(false)
{}

F0::F0(QFileInfo &path)
    : _path()
    , _data()
    , _fps(0)
    , _changed(false)
{
    openF0(path);
}

F0::~F0()
{
    if (!_data.empty()) {
        _data.clear();
    }
}

int F0::openF0(QFileInfo &path)
{
    if (!_data.empty()) {
        QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("既にデータが読み込まれています"));
        return 1;
    }
    if (!path.exists() || !path.isFile()) {
        QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1というファイルは存在しません。").arg(path.filePath()));
        return 1;
    }
    if (path.suffix().toLower() == "csv") {
        QFile file(path.filePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(path.filePath()));
            return 1;
        }
        QTextStream in(&file);
        //in.setEncoding(QStringConverter::Utf8);  // 必要に応じてエンコーディング指定
        QMap<double, double> map;
        while (!in.atEnd()) {
            QString line = in.readLine();         // 1行読み取り
            QStringList fields = line.split(','); // カンマで分割

            map[fields[0].toDouble()] = fields[1].toDouble();
        }
        for (int i = 0; i <= map.lastKey()*100; i+=1) {
            if (map.contains(i/100.0)) {
                _data.append(Note(map[i/100.0]));
            } else {
                _data.append(Note());
            }
        }

        file.close();
        _fps = 100;
    } else if (path.suffix().toLower() == "f0") {
        QFile file(path.filePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(path.filePath()));
            return 1;
        }
        while (!file.atEnd()) {
            QByteArray chunk = file.read(4);
            if (chunk.isEmpty()) {
                break;  // エラーまたはEOF
            }

            bool ok;
            float v(0.0);
            memcpy(&v, chunk.data(), 4);
            //if (ok) {
            //    QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1を読み込めませんでした。").arg(path.filePath()));
            //    file.close();
            //    return 1;
            //}
            _data.append(Note(v));
        }

        file.close();
        _fps = 200;
    } else {
        QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1はサポートされていません。").arg(path.suffix()));
        return 1;
    }
    _path = path;
    return 0;
}

int F0::getDataSize() const
{
    return _data.size();
}

Note F0::getData(int i) const
{
    if(0 < i && i < _data.size()) return _data.at(i);
    return Note();
}

QList<Note> F0::getData(int first, int size) const
{
    QList<Note> ret(size);
    for(int i(0); i+first < _data.size() && i < size; i++) ret.append(_data.at(i+first));
    return ret;
}

void F0::setData(int i, Note d)
{
    _changed = true;
    if(0 < i && i < _data.size()) _data[i] = d;
}

void F0::setData(int first, const QList<Note> &ds)
{
    _changed = true;
    if(0 < first && first < _data.size() && first + ds.size() < _data.size()) {
        for(int i(0); i < ds.size(); i++) _data[i+first] = ds.at(i);
    }
}

void F0::closeF0()
{
    if(_data.empty()) {
        return;
    } else {
        _data.clear();
    }
    _changed = false;
}

int F0::getFPS() const
{
    return _fps;
}

int F0::saveF0()
{
    if(_data.empty() || !_path.isFile()) return 1;
    if(_path.suffix().toLower() == "csv") {
        QFile file(_path.filePath());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(_path.filePath()));
            return 1;
        }
        QTextStream out(&file);
        int i(0);
        for(auto note: _data) {
            out << QObject::tr("%1,%2").arg(static_cast<double>(i)/_fps).arg(note.toHz());
            out << "\n";
            i++;
        }
        file.close();
        _changed = false;
        return 0;
    } else if(_path.suffix().toLower() == "f0") {
        QFile file(_path.filePath());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(_path.filePath()));
            return 1;
        }
        float ave(_data.at(0).toHz());
        bool n(false);
        for(auto note: _data) {
            float f(note.toHz());
            if(_fps == 100 && n) {
                if(ave > 0.0) {
                    ave += f;
                    ave /= 2.0;
                }
                if(f == 0.0) ave = 0.0;
                file.write(reinterpret_cast<const char *>(&ave), sizeof(float));
                ave = f;
            }
            file.write(reinterpret_cast<const char*>(&f), sizeof(float));
            n = true;
        }
        file.close();
        _changed = false;
        return 0;
    } else {
        return 1;
    }
}

int F0::saveF0as(QFileInfo &path)
{
    if(_data.empty() || path.isDir()) return 1;
    if(path.suffix().toLower() == "csv") {
        QFile file(path.filePath());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(_path.filePath()));
            return 1;
        }
        QTextStream out(&file);
        int i(0);
        for(auto note: _data) {
            out << QObject::tr("%1,%2").arg(static_cast<double>(i)/_fps).arg(note.toHz());
            out << "\n";
            i++;
        }
        file.close();
        _changed = false;
        _path = path;
        return 0;
    } else if(path.suffix().toLower() == "f0") {
        QFile file(path.filePath());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
            QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1が開けませんでした。").arg(_path.filePath()));
            return 1;
        }
        float ave(_data.at(0).toHz());
        bool n(false);
        for(auto note: _data) {
            float f(note.toHz());
            if(_fps == 100 && n) {
                if(ave > 0.0) {
                    ave += f;
                    ave /= 2.0;
                }
                if(f == 0.0) ave = 0.0;
                file.write(reinterpret_cast<const char *>(&ave), sizeof(float));
                ave = f;
            }
            file.write(reinterpret_cast<const char*>(&f), sizeof(float));
            n = true;
        }
        file.close();
        _changed = false;
        _path = path;
        return 0;
    } else {
        return 1;
    }
}

bool F0::isChanged() const
{
    return _changed;
}

QString F0::getFileName() const
{
    if(_data.empty()) return "";
    return _path.fileName();
}

QList<double> F0::getFreq(const double A4) const
{
    if(_data.empty()) return QList<double>();
    QList<double> ret;
    for(auto n: _data) {
        ret.append(n.toHz(A4));
    }
    return std::move(ret);
}
