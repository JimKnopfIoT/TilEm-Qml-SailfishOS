/****************************************************************************
**
** Copyright (C) 2009 Hugues Luc BRUANT aka fullmetalcoder 
**                    <non.deterministic.finite.organism@gmail.com>
** 
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define QT_NO_KEYWORDS

#include "settings.h"

#include "skinops.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPixmap>
#include <QFileInfo>
#include <QRect>
#include <QPoint>
#include <QBuffer>
#include <QImage>

/**
 * @brief Transforms an RECt emu into a QRect
 *
 * @param rect
 *
 * @return 
 */
QRect transform(RECT rect) {
//    qDebug() << rect.top << " " <<rect.right << " " << rect.bottom << " " << rect.left;
    return QRect(QPoint(rect.left,rect.top),
                 QPoint(rect.right,rect.bottom));
}

class SettingsPrivate : public SKIN_INFOS
{
};

/*!
	\file settings.cpp
	\brief Implementation of the Settings class
*/

/*!
	\class Settings
	\brief A small utility class to read settings
	
	Basically a parser that produces a settings tree from a string of a custom
	format and a couple of methods to access the data in that tree.
	
	The main (only) purpose of this class is to load skin files so do not expect
	it to be general purpose.
*/

Settings::Settings() : m_private(NULL), m_image(0)
{
}

Settings::~Settings()
{
    delete m_private;
    delete m_image;
}

int Settings::type() const
{
    if(valit())
        return m_private->type;
    return 0;
}

int Settings::width() const
{
    if(valit())
        return m_private->width;
    return 0;
}

int Settings::height() const
{
    if(valit())
        return m_private->height;
    return 0;
}

unsigned int Settings::colorType() const
{
    if(valit())
        return m_private->colortype;
    return 0;
}

unsigned int Settings::lcdBlack() const
{
    if(valit())
        return m_private->lcd_black;
    return 0;
}

unsigned int Settings::lcdWhite() const
{
    if(valit())
        return m_private->lcd_white;
    return 0;
}

ScaleFactor Settings::scale() const
{
    ScaleFactor sf;
    if(valit()) {
        sf.sx = m_private->sx;
        sf.sy = m_private->sy;
    }
    return sf;
}

QRect Settings::lcdPos() const
{
    if(valit())
        return transform(m_private->lcd_pos);
    return QRect();
}

QList<QRect> Settings::keysPos() const
{
    return m_keyPos;
}

QString Settings::name() const
{
    if(valit())
        return QString::fromLatin1(m_private->name);
    return QString();
}

QString Settings::author() const
{
    if(valit())
        return QString::fromLatin1(m_private->author);
    return QString();
}

long Settings::jpegOffset() const
{
    if(valit())
        return m_private->jpeg_offset;
    return 0.0;
}

QImage *Settings::image()
{
    return m_image;
}

/**
 * @brief Load a skin file
 *
 * @param file the skin file
 *
 * @return false on an error
 */
bool Settings::load(const QString& file)
{
    qDebug() << "Settings: load";
    qDebug() << "Settings: trying to load file:" << file;
    delete m_image;

    GError *err = NULL;
    m_private = new SettingsPrivate;
    int result = skin_load(m_private,file.toLocal8Bit().data(),&err);
    if(result != 0) {
        qDebug() << "Settings: couldn't load, error code:" << result;
        if(err) {
            qDebug() << "Settings: GError message:" << err->message;
            g_error_free(err);
        }
        return false;
    }
    qDebug() << "Settings: skin_load successful!";

    m_image = new QImage();

    GdkPixbuf* pixbuf = m_private->raw;
    const uchar* bdata = (const uchar*)gdk_pixbuf_get_pixels(pixbuf);
    QSize bsize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    int stride = gdk_pixbuf_get_rowstride(pixbuf);

    QImage im(bdata, bsize.width(), bsize.height(), stride, QImage::Format_RGB888 );
    *m_image = im.rgbSwapped();
    if(m_image->isNull()) {
         qDebug() << "Image Error";
         delete m_image;
         return false;
    }

    m_keyPos.clear();

    for(int iii = 0; iii < SKIN_KEYS; iii++) {
        QRect r = transform(m_private->keys_pos[iii]);
        if(!r.isValid())
            break;
        m_keyPos << r;
    }
    qDebug() << "Settings: loaded" << m_keyPos.length() << "key positions";
    if(m_keyPos.length() > 0) {
        qDebug() << "First key:" << m_keyPos[0];
        qDebug() << "Last key:" << m_keyPos[m_keyPos.length()-1];
    }

    return true;
}

/**
 * @brief See if the skin file is valit
 *
 * @return True is the skin is loaded
 */
bool Settings::valit() const
{
    if(m_private)
        return true;
    return false;
}

/**
 * @brief Key beneath a point
 *
 * @param x x value of the point
 * @param y y value of the point
 *
 * @return keyindex
 */
int Settings::keyIndex(int x, int y) const
{
    qDebug() << "get keyindex " << x << "x" << y;
    int ix, iy, nearest = -1, i;
    int dx, dy, d, best_d = G_MAXINT;

    ix = (x + 0.5);
    iy = (y + 0.5);

    for (i = 0; i < m_keyPos.length(); i++) {
        if (ix >= m_keyPos[i].left()
                && ix < m_keyPos[i].right()
                && iy >= m_keyPos[i].top()
                && iy < m_keyPos[i].bottom()) {
            dx = (m_keyPos[i].left() + m_keyPos[i].right()
                  - 2 * ix);
            dy = (m_keyPos[i].top() + m_keyPos[i].bottom()
                  - 2 * iy);
            d = ABS(dx) + ABS(dy);

            if (d < best_d) {
                best_d = d;
                nearest = i;
            }
        }
    }
    return nearest;
}

#undef QT_NO_KEYWORDS
