#include "calcscreen.h"

#include <QThread>
#include <QPainter>
#include <QDebug>

#include "calc.h"

CalcScreen::CalcScreen(QQuickItem *parent) :
    QQuickPaintedItem(parent), m_calc(NULL), m_lcdTimerId(0), m_lcdW(0), m_lcdH(0)
{
    // FIX (Oct 13, 2025): CalcScreen is visual-only, must NOT handle mouse/touch events
    // This ensures events pass through to MouseArea siblings at higher z-index
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);

//    setFillColor(Qt::color1);
    connect(this, SIGNAL( widthChanged() ), this, SLOT( setLcd() ));
    connect(this, SIGNAL( heightChanged() ), this, SLOT( setLcd() ));
}

CalcScreen::~CalcScreen()
{
}

void CalcScreen::paint(QPainter *painter)
{
    QRectF target = contentsBoundingRect();
    QRectF source(0.0, 0.0, m_lcdW, m_lcdH);
    painter->drawImage(target,m_screen,source);
}

Calc *CalcScreen::calc() const
{
    return m_calc;
}

void CalcScreen::setLcd()
{
    if(m_lcdW == (int)width() && m_lcdH == (int)height())
        return;
    m_lcdW = (int)width();
    m_lcdH = (int)height();
    qDebug() << "setLcd: " << m_lcdW << "x" << m_lcdH;
    m_screen = QImage(m_lcdW,m_lcdH, QImage::Format_RGB32);
    m_screen.fill(Qt::black);  // Initialize with black instead of random data

    // FIX (Oct 14, 2025 21:42 UTC): Don't call updateLCD() during initialization
    // setLcd() is called from width/heightChanged signals during QML construction
    // At that point m_calc object might exist but internal TilEm calc not initialized
    // updateLCD() will be called by the timer once ROM is loaded via fileLoaded()
    // Don't try to render until ROM is actually loaded
    qDebug() << "setLcd: Skipping updateLCD() - will be called by timer after ROM load";
}

void CalcScreen::fileLoaded()
{
    qDebug() << "fileLoaded - starting LCD timer";
    qDebug() << "CalcScreen dimensions:" << width() << "x" << height();
    qDebug() << "m_lcdW:" << m_lcdW << "m_lcdH:" << m_lcdH;
    qDebug() << "Item flags - visible:" << isVisible() << "enabled:" << isEnabled();

    // Ensure LCD dimensions are set before starting timer
    if(m_lcdW <= 0 || m_lcdH <= 0) {
        qDebug() << "LCD dimensions not set, calling setLcd()";
        setLcd();
    }

    // Kill old timer if exists
    if(m_lcdTimerId > 0) {
        qDebug() << "Killing old timer:" << m_lcdTimerId;
        killTimer(m_lcdTimerId);
    }

    // start LCD update timer
    m_lcdTimerId = startTimer(10);
    qDebug() << "LCD timer ID:" << m_lcdTimerId;

    // Force an immediate update
    qDebug() << "Forcing immediate LCD update";
    updateLCD();
}

void CalcScreen::beforeFileLoaded()
{
    killTimer(m_lcdTimerId);
}

void CalcScreen::timerEvent(QTimerEvent *e)
{
    if ( e->timerId() == m_lcdTimerId )
        updateLCD();
    else
        QQuickPaintedItem::timerEvent(e);
}

void CalcScreen::setCalc(Calc *arg)
{
    if (m_calc != arg) {
        delete m_calc;
        m_calc = arg;
        connect(m_calc, SIGNAL(loaded()), this, SLOT( fileLoaded() ));
        connect(m_calc, SIGNAL(beginLoad()), this, SLOT( beforeFileLoaded() ));
        emit calcChanged(arg);

        // Start LCD timer immediately if calc is already loaded
        if(m_calc && m_calc->isValid()) {
            qDebug() << "Calc already loaded, starting LCD timer now";
            fileLoaded();
        }
    }
}

void CalcScreen::takeScreenshot()
{

}

void CalcScreen::updateLCD()
{
    if(!m_calc)
        return;

    // lcdUpdate() has a side effect: it refreshes the internal LCD pixel
    // buffer via the hardware get_lcd callback, so it must be called even
    // though we always render regardless of its return value.
    m_calc->lcdUpdate();

    if(m_lcdW <= 0 || m_lcdH <= 0)
        return;

    const int w = m_calc->lcdWidth();
    const int h = m_calc->lcdHeight();
    const unsigned int *cd = m_calc->lcdData();

    QRgb *d = reinterpret_cast<QRgb*>(m_screen.bits());

    // write (scaled) LCD into screen image
    for ( int i = 0; i < m_lcdH; ++i )
    {
        for ( int j = 0; j < m_lcdW; ++j )
        {
            int y = (h * i) / m_lcdH;
            int x = (w * j) / m_lcdW;

            d[i * m_lcdW + j] = cd[y * w + x];
        }
    }

    update(boundingRect().toAlignedRect());
}
