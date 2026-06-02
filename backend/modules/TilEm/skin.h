#ifndef SKIN_H
#define SKIN_H

#include <QObject>
#include <QPoint>
#include <QPolygon>

#include "settings.h"

class QMouseEvent;
class QPoint;
class QImage;
class QPixmap;
class SkinImage;
class SkinImage;

class Skin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString skinFile READ skinFile WRITE setSkinFile NOTIFY skinFileChanged)
    Q_PROPERTY(int lcdX READ getLcdX NOTIFY lcdXChanged)
    Q_PROPERTY(int lcdY READ getLcdY NOTIFY lcdYChanged)
    Q_PROPERTY(int lcdH READ getLcdH NOTIFY lcdHChanged)
    Q_PROPERTY(int lcdW READ getLcdW NOTIFY lcdWChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int scaleX READ scaleX NOTIFY scaleXChanged)
    Q_PROPERTY(int scaleY READ scaleY NOTIFY scaleYChanged)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString author READ author NOTIFY authorChanged)
public:
    explicit Skin(QObject *parent = 0);
    ~Skin();

    QString skinFile() const
    {
        return m_skinFile;
    }

    int getLcdX() const
    {
        return m_settings.lcdPos().left();
    }

    int getLcdY() const
    {
        return m_settings.lcdPos().top();
    }

    int getLcdH() const
    {
        return m_settings.lcdPos().height();
    }

    int getLcdW() const
    {
        return m_settings.lcdPos().width();
    }

    int height() const
    {
        return m_settings.height();
    }

    int width() const
    {
        return m_settings.width();
    }

    QString name() const
    {
        return m_settings.name();
    }

    QString author() const
    {
        return m_settings.author();
    }

    QImage *getSkinImage();

    int scaleX() {
        return m_settings.scale().sx;
    }

    int scaleY() {
        return m_settings.scale().sy;
    }

    Q_INVOKABLE int keyCode(int x, int y);
    Q_INVOKABLE int getCode(int key);
    Q_INVOKABLE int getButtonCount();
    Q_INVOKABLE int getButtonX(int index);
    Q_INVOKABLE int getButtonY(int index);
    Q_INVOKABLE int getButtonWidth(int index);
    Q_INVOKABLE int getButtonHeight(int index);

    void setSkinImage(SkinImage *si);

Q_SIGNALS:

    void skinFileChanged(QString arg);
    void lcdXChanged(int);
    void lcdYChanged(int);
    void lcdHChanged(int);
    void lcdWChanged(int);
    void lcdChanged();

    void skinChanged();
    void skinSettingsLoaded();

    void heightChanged(int);
    void widthChanged(int);
    void authorChanged(QString);
    void nameChanged(QString);

    void scaleXChanged(int);
    void scaleYChanged(int);

public slots:
    void setSkinFile(QString arg);
    void loadSkin();
    void emitLcdChanged();

private:

    QString m_skinFile;
    Settings m_settings;
    SkinImage* m_si;
};

#endif // SKIN_H
