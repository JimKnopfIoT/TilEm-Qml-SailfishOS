#include "calcscreen.h"

#include <QThread>
#include <QPainter>
#include <QDebug>

#include <cstring>

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
    if (m_screen.isNull())
        return;

    // m_screen holds the calculator's native-resolution LCD (e.g. 96x64).
    // Scale it up to the item here (nearest-neighbour = crisp pixels) on the
    // render thread, instead of scaling per-pixel on the GUI thread every frame.
    painter->drawImage(contentsBoundingRect(), m_screen, m_screen.rect());
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
    // m_screen is allocated at the native LCD resolution by updateLCD(); the item
    // size (m_lcdW/m_lcdH) only drives the scale done in paint(). Nothing to do
    // here on resize - paint() rescales the existing native image automatically.

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

    // start LCD update timer (~30 Hz is plenty for a calculator LCD; 100 Hz
    // wasted CPU on the GUI thread and hurt touch latency)
    m_lcdTimerId = startTimer(33);
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

    // lcdUpdate() refreshes the composite LCD buffer via the hardware get_lcd
    // callback and returns whether the pixels actually changed this frame.
    const bool changed = m_calc->lcdUpdate();

    const int w = m_calc->lcdWidth();
    const int h = m_calc->lcdHeight();
    const unsigned int *cd = m_calc->lcdData();
    if(w <= 0 || h <= 0 || !cd)
        return;

    // Keep the image at the NATIVE LCD resolution and let paint() scale it.
    // (Re)allocate only when the source size changes; otherwise skip the whole
    // frame when nothing changed - an idle calculator then costs almost nothing.
    if(m_screen.width() != w || m_screen.height() != h)
        m_screen = QImage(w, h, QImage::Format_RGB32);
    else if(!changed)
        return;

    // Composite buffer is already ARGB (qRgb), matching Format_RGB32 - just copy.
    memcpy(m_screen.bits(), cd, static_cast<size_t>(w) * h * sizeof(quint32));

    update();
}
