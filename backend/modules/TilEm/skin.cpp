#include "skin.h"

#include "skinimage.h"

#include <QDebug>
#include <QMouseEvent>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QRect>
#include "skinimage.h"

#include <scancodes.h>

/* Table for translating skin-file key number (based on actual
   position, and defined by the VTI/TiEmu file formats) into a
   scancode.  Note that the TILEM_KEY_* constants are named according
   to the TI-83 keypad layout; other models use different names for
   the keys, but the same scancodes. */
static const int keycode_map[] =
    { TILEM_KEY_YEQU,
      TILEM_KEY_WINDOW,
      TILEM_KEY_ZOOM,
      TILEM_KEY_TRACE,
      TILEM_KEY_GRAPH,

      TILEM_KEY_2ND,
      TILEM_KEY_MODE,
      TILEM_KEY_DEL,
      TILEM_KEY_LEFT,
      TILEM_KEY_RIGHT,
      TILEM_KEY_UP,
      TILEM_KEY_DOWN,
      TILEM_KEY_ALPHA,
      TILEM_KEY_GRAPHVAR,
      TILEM_KEY_STAT,

      TILEM_KEY_MATH,
      TILEM_KEY_MATRIX,
      TILEM_KEY_PRGM,
      TILEM_KEY_VARS,
      TILEM_KEY_CLEAR,

      TILEM_KEY_RECIP,
      TILEM_KEY_SIN,
      TILEM_KEY_COS,
      TILEM_KEY_TAN,
      TILEM_KEY_POWER,

      TILEM_KEY_SQUARE,
      TILEM_KEY_COMMA,
      TILEM_KEY_LPAREN,
      TILEM_KEY_RPAREN,
      TILEM_KEY_DIV,

      TILEM_KEY_LOG,
      TILEM_KEY_7,
      TILEM_KEY_8,
      TILEM_KEY_9,
      TILEM_KEY_MUL,

      TILEM_KEY_LN,
      TILEM_KEY_4,
      TILEM_KEY_5,
      TILEM_KEY_6,
      TILEM_KEY_SUB,

      TILEM_KEY_STORE,
      TILEM_KEY_1,
      TILEM_KEY_2,
      TILEM_KEY_3,
      TILEM_KEY_ADD,

      TILEM_KEY_ON,
      TILEM_KEY_0,
      TILEM_KEY_DECPNT,
      TILEM_KEY_CHS,
      TILEM_KEY_ENTER };
const int keycode_map_length = 50;

Skin::Skin(QObject *parent) :
    QObject(parent), m_si(NULL)
{
    connect(this,SIGNAL(lcdChanged()),this,SLOT(emitLcdChanged()));
    connect(this,SIGNAL(skinSettingsLoaded()), this, SLOT(loadSkin()));
}

Skin::~Skin()
{
}

QImage *Skin::getSkinImage()
{
    return m_settings.image();
}

int Skin::keyCode(int x, int y)
{
    qDebug() << "get keycode " << x << "x" << y;
    return getCode(m_settings.keyIndex(x,y));
}

/**
 * @brief Transloate tilem skin keycode to emulator scancode
 *
 * @param key keycode
 *
 * @return Scancode 
 */
int Skin::getCode(int key)
{
    qDebug() << "getcode " << key;
    if(key > -1 && key<keycode_map_length)
        return keycode_map[key];
    return -1;
}

int Skin::getButtonCount()
{
    return m_settings.keysPos().length();
}

int Skin::getButtonX(int index)
{
    QList<QRect> keys = m_settings.keysPos();
    if(index >= 0 && index < keys.length())
        return keys[index].x();
    return 0;
}

int Skin::getButtonY(int index)
{
    QList<QRect> keys = m_settings.keysPos();
    if(index >= 0 && index < keys.length())
        return keys[index].y();
    return 0;
}

int Skin::getButtonWidth(int index)
{
    QList<QRect> keys = m_settings.keysPos();
    if(index >= 0 && index < keys.length())
        return keys[index].width();
    return 0;
}

int Skin::getButtonHeight(int index)
{
    QList<QRect> keys = m_settings.keysPos();
    if(index >= 0 && index < keys.length())
        return keys[index].height();
    return 0;
}

void Skin::setSkinImage(SkinImage *si)
{
    m_si = si;
}

void Skin::setSkinFile(QString arg)
{
    arg.remove(QRegExp("^\\w*://"));
    if (m_skinFile != arg) {
        qDebug() << "set skin file " << arg;
        m_skinFile = arg;
        qDebug() << "load skin settings";
        bool result = m_settings.load(arg);
        if(!result) {
            qDebug() << "skin settings failed";
        }
        else {
            qDebug() << "skin settings loaded";
            emit skinSettingsLoaded();
        }
        emit skinFileChanged(arg);
    }
}

void Skin::loadSkin()
{
    qDebug("loading skin");
    emit skinChanged();
    emit lcdChanged();
    emit heightChanged(height());
    emit widthChanged(width());
    emit nameChanged(name());
    emit authorChanged(author());
    emit scaleXChanged(scaleX());
    emit scaleYChanged(scaleY());
}

void Skin::emitLcdChanged()
{
    qDebug() << "Skin::emitLcdChanged() - X:" << getLcdX() << "Y:" << getLcdY() << "W:" << getLcdW() << "H:" << getLcdH();
    emit lcdXChanged(getLcdX());
    emit lcdYChanged(getLcdY());
    emit lcdWChanged(getLcdW());
    emit lcdHChanged(getLcdH());
}
