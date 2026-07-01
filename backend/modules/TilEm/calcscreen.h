#ifndef CALCSCREEN_H
#define CALCSCREEN_H

#include <QQuickPaintedItem>
#include <QImage>

class Settings;

class Calc;
class CalcLink;

class CalcScreen : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(Calc* calc READ calc WRITE setCalc NOTIFY calcChanged)

public:
    CalcScreen(QQuickItem *parent = 0);
    ~CalcScreen();
    void paint(QPainter *painter);

    Calc* calc() const;

//    virtual QSize sizeHint() const;

    bool isPaused() const;

    float scale() const;

Q_SIGNALS:
    void calcChanged(Calc* calc);

    void paused();
    void resumed();

    void paused(bool isPaused);

    void externalLinkGrabbed(bool externalLinkGrabbed);

public slots:
    void setCalc(Calc* arg);

    void takeScreenshot();

    void updateLCD();

    void setLcd();

    void fileLoaded();
    void beforeFileLoaded();



protected:
    void timerEvent(QTimerEvent *e);

private:
    Calc* m_calc;
    QImage m_screen;

    int m_lcdX, m_lcdY, m_lcdW, m_lcdH;

    int m_lcdTimerId;
};

#endif // CALCSCREEN_H
