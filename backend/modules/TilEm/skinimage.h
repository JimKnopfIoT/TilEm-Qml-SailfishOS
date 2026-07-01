#ifndef SKINIMAGE_H
#define SKINIMAGE_H

#include <QQuickPaintedItem>

class Skin;

class SkinImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* skin READ skin WRITE setSkin NOTIFY skinChanged)
    Q_PROPERTY(float scaleX READ scaleX WRITE setScaleX NOTIFY scaleXChanged)
    Q_PROPERTY(float scaleY READ scaleY WRITE setScaleY NOTIFY scaleYChanged)
    Q_PROPERTY(float scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(int drawWidth READ drawWidth NOTIFY drawWidthChanged)
    Q_PROPERTY(int drawHeight READ drawHeight NOTIFY drawHeightChanged)
    Q_PROPERTY(bool keepAspect READ keepAspect WRITE setKeepAspect NOTIFY keepAspectChanged)
    Q_PROPERTY(int lcdX READ lcdX NOTIFY lcdXChanged)
    Q_PROPERTY(int lcdY READ lcdY NOTIFY lcdYChanged)
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY hasImageChanged)

public:
    explicit SkinImage(QQuickItem *parent = 0);
    void setImage(QImage *image);

    void paint(QPainter *painter);

    QObject *skin();

    float scale() const
    {
        if(m_scaleX == m_scaleY)
            return m_scaleX;
        return -1;
    }

    float scaleX() const
    {
        return m_scaleX;
    }

    float scaleY() const
    {
        return m_scaleY;
    }

    int drawWidth() const
    {
        return m_drawWidth;
    }

    int drawHeight() const
    {
        return m_drawHeight;
    }

    bool keepAspect() const
    {
        return m_kAspect;
    }

    int lcdX() const
    {
        return m_lcdX;
    }

    int lcdY() const
    {
        return m_lcdY;
    }

    bool hasImage() const
    {
        return m_hasImage;
    }

    Q_INVOKABLE int xOffset();
    Q_INVOKABLE int yOffset();

    Q_INVOKABLE int normalizeX(int x);
    Q_INVOKABLE int normalizeY(int y);

    Q_INVOKABLE int translateX(int x);
    Q_INVOKABLE int translateY(int y);

Q_SIGNALS:
    void skinChanged(QObject*);
    void scaleChanged(float);
    void scaleXChanged(float);
    void scaleYChanged(float);
    void keepAspectChanged(bool);
    void drawHeightChanged(int);
    void drawWidthChanged(int);
    void lcdXChanged(int);
    void lcdYChanged(int);
    void hasImageChanged(bool);

public slots:
    void setSkin(QObject *s);
    void resetScale();
    void setScaleX(const float arg);
    void setScaleY(const float arg);
    void setScale(const float arg);
    void setKeepAspect(const int arg);
    void setDrawWidth(const int arg);
    void setDrawHeight(const int arg);

    void setLcdX(int arg);
    void setLcdY(int arg);
    void setHasImage(bool arg);

    void setLcdXFromNormalized(int arg);
    void setLcdYFromNormalized(int arg);

    void loadImage();
private:
    QImage* m_image;

    Skin* m_s;
    float m_scaleX;
    float m_scaleY;
    int m_drawWidth;
    int m_drawHeight;

    int m_lcdX;
    int m_lcdY;
    bool m_hasImage;

    bool m_kAspect;
};

#endif // SKINIMAGE_H
