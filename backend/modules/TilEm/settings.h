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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/*!
	\file settings.h
	\brief Definition of the Settings class
*/

#include <QList>
#include <QStringList>
#include <QRect>

class SettingsPrivate;
class QPixmap;
class QImage;

struct ScaleFactor {
    int sx;
    int sy;
};

class Settings
{
    public:
		Settings();
        ~Settings();

        int type() const;
        int width() const;
        int height() const;
        unsigned int colorType() const;
        unsigned int lcdBlack() const;
        unsigned int lcdWhite() const;

        ScaleFactor scale() const;

        QRect lcdPos() const;
        QList<QRect> keysPos() const;

        QString name() const;
        QString author() const;

        long jpegOffset() const;

        QImage* image();

        bool load(const QString& file);

        bool valit() const;

        int keyIndex(int x, int y) const;
		
    private:
        SettingsPrivate *m_private;
        QImage* m_image;
        QList<QRect> m_keyPos;
};

#endif
