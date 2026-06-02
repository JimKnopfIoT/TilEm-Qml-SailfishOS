#include "skinimage.h"

#include "skin.h"

#include <QPainter>
#include <QDebug>
#include <QImage>

SkinImage::SkinImage(QQuickItem *parent) :
    QQuickPaintedItem(parent), m_image(NULL), m_s(NULL), m_scaleX(1.0), m_scaleY(1.0), m_lcdX(0), m_lcdY(0), m_hasImage(false), m_kAspect(true)
{
    // FIX (Oct 13, 2025): SkinImage is visual-only, must NOT handle mouse/touch events
    // This ensures events pass through to MouseArea siblings at higher z-index
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);

    connect(this,SIGNAL(skinChanged(QObject*)),this,SLOT(loadImage()));
    connect(this,SIGNAL(widthChanged()),this,SLOT(resetScale()));
    connect(this,SIGNAL(heightChanged()),this,SLOT(resetScale()));
    connect(this,SIGNAL(scaleXChanged(float)),this,SLOT(update()));
    connect(this,SIGNAL(scaleYChanged(float)),this,SLOT(update()));
}

/**
 * @brief Load image from the skin
 */
void SkinImage::loadImage()
{
    qDebug() << "load image";
    if(m_s && m_s->getSkinImage())
        setImage(m_s->getSkinImage());
}

/**
 * @brief Set the background image
 *
 * @param image
 */
void SkinImage::setImage(QImage *image)
{
    qDebug("set image");
    if(!image || image->isNull()) {
        setHasImage(false);
        return;
    }
    m_image = image;
    setHasImage(true);
    setImplicitWidth(m_image->width());
    setImplicitHeight(m_image->height());
    qDebug() << "w: " << m_image->width() << "h: " << m_image->height();
    resetScale();
}

void SkinImage::paint(QPainter *painter)
{
    if(hasImage() && m_image && !m_image->isNull()) {
        QRect target(xOffset(),yOffset(), m_drawWidth, m_drawHeight);
        QRect source(0, 0, m_image->width(), m_image->height());
        painter->drawImage(target,*m_image,source);
    }
}

QObject *SkinImage::skin()
{
    if(m_s)
        return dynamic_cast<QObject*>(m_s);
    return new QObject();
}

/**
 * @brief The x-offset of the skin image to the total when cropping
 *
 * @return 
 */
int SkinImage::xOffset()
{
    return ((int)width()-m_drawWidth)/2;
}

/**
 * @brief The y-offset of the skin image to the total when cropping
 *
 * @return 
 */
int SkinImage::yOffset()
{
    return ((int)height()-m_drawHeight)/2;
}

/**
 * @brief Normalize the x position from the screen to the skinfile
 *
 * @param x
 *
 * @return 
 */
int SkinImage::normalizeX(int x)
{
    int result = (float)(x - xOffset())/m_scaleX;
    qDebug() << "normalizeX: input=" << x << "xOffset=" << xOffset() << "scaleX=" << m_scaleX << "result=" << result;
    return result;
}

/**
 * @brief Normalize the y position from the screen to the skinFile
 *
 * @param y
 *
 * @return 
 */
int SkinImage::normalizeY(int y)
{
    int result = (float)(y - yOffset())/m_scaleY;
    qDebug() << "normalizeY: input=" << y << "yOffset=" << yOffset() << "scaleY=" << m_scaleY << "result=" << result;
    return result;
}

/**
 * @brief Translate the x position on from the skinfile to the screen
 *
 * @param x
 *
 * @return 
 */
int SkinImage::translateX(int x)
{
    return xOffset() + (float)x*m_scaleX;
}

/**
 * @brief Translate the y position on from the skinfile to the screen
 *
 * @param y
 *
 * @return 
 */
int SkinImage::translateY(int y)
{
    // FIX (Oct 13, 2025): Was using m_scaleX instead of m_scaleY - broke LCD Y positioning!
    return yOffset() + (float)y*m_scaleY;
}

void SkinImage::setSkin(QObject *s)
{
    if(!s)
        return;
    m_s = dynamic_cast<Skin*>(s);
    if(m_s == NULL)
        return;
    connect(m_s,SIGNAL(skinChanged()),this,SLOT(loadImage()));
    m_s->setSkinImage(this);
    connect(m_s, SIGNAL(lcdXChanged(int)), this, SLOT(setLcdXFromNormalized(int)));
    connect(m_s, SIGNAL(lcdYChanged(int)), this, SLOT(setLcdYFromNormalized(int)));
    emit skinChanged(s);
}

/**
 * @brief Reset the scale to fit the screen
 *      If keeping aspect is true, then it will make it crop
 */
void SkinImage::resetScale()
{
    if(!hasImage()) {
        setScale(1.0);
        return;
    }
    if(m_kAspect) {
        float sx = (float)width()/(float)(m_image->width());
        float sy = (float)height()/(float)(m_image->height());
        setScale(std::min(sx,sy));
    }
    else {
        setScaleX((float)width()/(float)(m_image->width()));
        setScaleY((float)height()/(float)(m_image->height()));
    }
    if(!m_s)
        return;
    setLcdXFromNormalized(m_s->getLcdX());
    setLcdYFromNormalized(m_s->getLcdY());
    update();
}

void SkinImage::setScaleX(const float arg)
{
    if(m_scaleX != arg) {
        qDebug() << "SkinImage::setScaleX() - old:" << m_scaleX << "new:" << arg;
        m_scaleX = arg;
        if(hasImage())
            setDrawWidth((float)m_image->width()*m_scaleX);
        emit scaleXChanged(arg);
        if(arg == m_scaleY)
            emit scaleChanged(arg);
    }
}

void SkinImage::setScaleY(const float arg)
{
    if(m_scaleY != arg) {
        qDebug() << "SkinImage::setScaleY() - old:" << m_scaleY << "new:" << arg;
        m_scaleY = arg;
        if(hasImage())
            setDrawHeight((float)m_image->height()*m_scaleY);
        emit scaleYChanged(arg);
        if(arg == m_scaleX)
            emit scaleChanged(arg);
    }
}

void SkinImage::setScale(const float arg)
{
    if(m_kAspect) {
        setScaleX(arg);
        setScaleY(arg);
    }
}

void SkinImage::setKeepAspect(const int arg)
{
    if(arg != m_kAspect) {
        m_kAspect = arg;
        emit keepAspectChanged(arg);
    }
}

void SkinImage::setDrawWidth(const int arg)
{
    if(m_drawWidth != arg) {
        m_drawWidth = arg;
        emit drawWidthChanged(arg);
    }
}

void SkinImage::setDrawHeight(const int arg)
{
    if(m_drawHeight!= arg) {
        m_drawHeight = arg;
        emit drawHeightChanged(arg);
    }
}

void SkinImage::setLcdX(int arg)
{
    if(arg != m_lcdX) {
        m_lcdX = arg;
        emit lcdXChanged(arg);
    }
}

void SkinImage::setLcdY(int arg)
{
    if(arg != m_lcdY) {
        m_lcdY = arg;
        emit lcdYChanged(arg);
    }
}

void SkinImage::setHasImage(bool arg)
{
    if(arg != m_hasImage) {
        m_hasImage = arg;
        emit hasImageChanged(arg);
    }
}

void SkinImage::setLcdXFromNormalized(int arg)
{
    setLcdX(translateX(arg));
}

void SkinImage::setLcdYFromNormalized(int arg)
{
    setLcdY(translateY(arg));
}
