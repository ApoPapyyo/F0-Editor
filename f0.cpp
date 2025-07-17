#include "f0.h"
#include <QDebug>
#include <QMessageBox>
#include <QObject>
F0::F0()
    : _data()
{}

int F0::openF0(QFileInfo &path)
{
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
    }
}
