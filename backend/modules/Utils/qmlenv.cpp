#include "qmlenv.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>

QmlEnv::QmlEnv(QObject *parent) :
    QObject(parent)
{
}

QmlEnv::~QmlEnv()
{
}

QString QmlEnv::readXdg(const QString type) {
    QProcess proc;
    proc.start( "xdg-user-dir", QStringList() << type );
    if (!proc.waitForStarted() || !proc.waitForFinished())
        return QString();
    return QString::fromLocal8Bit( proc.readAll()).trimmed();
}

QString QmlEnv::readEnvVar(const QString var) {
    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    return pe.value(var);
    QByteArray env = qgetenv(var.toStdString().c_str());
    return QString(env);
}
