#include "f0.h"
#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include <QMap>
F0::F0()
    : _path()
    , _data()
{}

F0::F0(QFileInfo &path)
    : _path()
    , _data()
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
        in.setEncoding(QStringConverter::Utf8);  // 必要に応じてエンコーディング指定
        QMap<double, double> map;
        while (!in.atEnd()) {
            QString line = in.readLine();         // 1行読み取り
            QStringList fields = line.split(','); // カンマで分割

            map[fields[0].toDouble()] = fields[1].toDouble();
        }
        for (double i = 0; i <= map.lastKey(); i+=0.01) {
            if (map.contains(i)) {
                _data.append(Note(map[i]));
            } else {
                _data.append(Note());
            }
        }

        file.close();
    } else if (path.suffix().toLower() == "lf0") {
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
            double v(chunk.toDouble(&ok));
            if (!ok) {
                QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1を読み込めませんでした。").arg(path.filePath()));
                file.close();
                return 1;
            }
            _data.append(Note(v));
        }

        file.close();
    } else {
        QMessageBox::critical(nullptr, QObject::tr("エラー"), QObject::tr("%1はサポートされていません。").arg(path.suffix()));
        return 1;
    }
    _path = path;
    return 0;
}

int F0::getDataSize() const
{
    return _data.count();
}
