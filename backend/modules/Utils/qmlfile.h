#ifndef QMLFILE_H
#define QMLFILE_H

#include <QObject>

class QmlFile : public QObject
{
    Q_OBJECT
public:
    explicit QmlFile(QObject *parent = 0);

    Q_INVOKABLE static bool removeFile(QString file);

    Q_INVOKABLE static bool mkdir(const QString dir);
    Q_INVOKABLE static bool fileExists(const QString file);

signals:

public slots:
};

#endif // QMLFILE_H
