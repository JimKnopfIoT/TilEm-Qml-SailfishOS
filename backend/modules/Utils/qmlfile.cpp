#include "qmlfile.h"
#include <QDir>
#include <QFile>

QmlFile::QmlFile(QObject *parent) : QObject(parent)
{

}

bool QmlFile::removeFile(QString file)
{
    QString tmp_file = file;
    tmp_file.remove(QRegExp("^\\w*://"));
    QFile fileObj(tmp_file);
    if(!fileObj.exists()) {
        return true;
    }
    return fileObj.remove();
}

bool QmlFile::mkdir(const QString dir) {
    QString tmp_dir = dir;
    tmp_dir.remove(QRegExp("^\\w*://"));
    if(QDir(tmp_dir).exists()) {
        return true;
    }
    return QDir().mkdir(tmp_dir);
}

bool QmlFile::fileExists(const QString file) {
    QString tmp_file = file;
    tmp_file.remove(QRegExp("^\\w*://"));
    return QFile(tmp_file).exists();
}
