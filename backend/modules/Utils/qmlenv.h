#ifndef QMLENV_H
#define QMLENV_H

#include <QObject>

class QmlEnv: public QObject
{
    Q_OBJECT
public:
    explicit QmlEnv(QObject *parent = 0);
    ~QmlEnv();

    Q_INVOKABLE static QString readXdg(const QString type);
	Q_INVOKABLE static QString readEnvVar(const QString var);
};

#endif // QMLENV_H
