#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include "calc.h"
#include "config.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

/*!
    \class CalcThread
    \brief Utility class to manage calc emulation

    Perform the emulation in a separate thread to reduce latency in both GUI and emulator.

    Uses desktop TilEm2's adaptive timing algorithm for optimal performance:
    - Throttles CPU when running faster than real-time
    - No sleep penalty when running slower than real-time
    - Auto-resyncs if falling behind by >100ms
*/
class CalcThread : public QThread
{
    Q_OBJECT

    public:
        CalcThread(Calc *c, QObject * p = 0);

        void step();

        void stop();

        void wakeUp();

    Q_SIGNALS:
        void runningChanged(bool y);

    protected:
        virtual void run();

    private:
        Calc *m_calc;
        volatile int exiting;

        QMutex m_sleepMutex;
        QWaitCondition m_wakeCondition;

        // Desktop TilEm2-style adaptive timing
        QElapsedTimer m_timer;           // High-precision timer
        qint64 m_idealTime;              // Ideal completion time (microseconds)
        bool m_limitSpeed;               // Whether to throttle speed
};

#endif // CALCTHREAD_H
