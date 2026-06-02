#ifndef QMLLOGGER_H
#define QMLLOGGER_H

#include <QObject>
#include <QDebug>

class QmlLogger : public QObject
{
    Q_OBJECT
public:
    explicit QmlLogger(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void log(const QString &message) {
        qDebug() << "[QML]" << message;
    }

    Q_INVOKABLE void debug(const QString &message) {
        qDebug() << "[QML DEBUG]" << message;
    }

    Q_INVOKABLE void warn(const QString &message) {
        qWarning() << "[QML WARN]" << message;
    }

    Q_INVOKABLE void error(const QString &message) {
        qCritical() << "[QML ERROR]" << message;
    }
};

#endif // QMLLOGGER_H
