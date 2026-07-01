#include "calcthread.h"

#include <QDebug>

CalcThread::CalcThread(Calc *c, QObject * p)
    : QThread(p), m_calc(c), exiting(0), m_idealTime(0), m_limitSpeed(true)
{

}

void CalcThread::step()
{
    if ( isRunning() )
        return;

    exiting = 1;
    start();
}

void CalcThread::stop()
{
//  stop emulator thread before loading a new ROM
    exiting = 1;

//  wait for thread to terminate
    while ( isRunning() )
        usleep(1000);
}

void CalcThread::wakeUp()
{
    // Wake up the emulator thread when an event occurs (key press, etc.)
    m_wakeCondition.wakeOne();
}

void CalcThread::run()
{
    int res;
    int loopCount = 0;
    const int TIMEOUT = 50000;  // 50ms per iteration (20 Hz target) - increased for ARM64 to allow interrupt handlers to complete
    const int RESYNC_THRESHOLD = 100000;  // Resync if >100ms behind (desktop TilEm2 value)

    qDebug() << "CalcThread: start running with desktop TilEm2 adaptive timing + sleep detection";

    emit runningChanged(true);

    // Start high-precision timer (desktop TilEm2: g_timer_start)
    m_timer.start();
    m_idealTime = 0;

    forever
    {
        // ARM64 FIX (Oct 9, 17:30 UTC): Sleep detection fix for TI-83 ROM boot
        // CRITICAL: The TI-83 ROM uses HALT instruction as "wait for interrupt", not "sleep mode"
        //
        // TI-83 ROM boot sequence:
        //   1. ROM boots and initializes hardware
        //   2. ROM executes HALT at PC=0x07DD to wait for timer interrupt
        //   3. Timer fires (~140Hz), wakes CPU, ROM continues execution
        //   4. ROM draws home screen and enters main loop
        //
        // OLD BUG: isAsleep() returned TRUE when ROM executed HALT at 0x07DD
        //   → CalcThread stopped emulator and waited for key press
        //   → Timers never fired (emulator stopped!)
        //   → ROM stuck at HALT forever
        //   → LCD remained blank, buttons didn't work
        //
        // FIX: Only sleep when poweronhalt=1 (TRUE sleep mode set by ROM via PORT3 bit 3)
        //   - Boot HALT (poweronhalt=0): Keep emulator running → timers fire → ROM progresses
        //   - User sleep (poweronhalt=1): Pause emulator → save battery → wait for key press
        //
        // Desktop TilEm2 sleep detection (emucore.c:calc_asleep + g_cond_wait)
        // Check if calculator is in TRUE sleep mode: halted CPU, no pending interrupts, AND poweronhalt=1
        if (m_calc->isAsleep() && m_calc->getPowerOnHalt()) {
            qDebug() << "CalcThread: calculator in TRUE sleep mode (poweronhalt=1), waiting for wake event";

            // Wait for wake-up signal (key press, etc.)
            m_sleepMutex.lock();
            m_wakeCondition.wait(&m_sleepMutex);
            m_sleepMutex.unlock();

            // Resync timer after waking up
            m_timer.start();
            m_idealTime = 0;

            qDebug() << "CalcThread: woken up from sleep mode, resuming emulation";
            continue;
        }

        // Run emulator for requested time (desktop TilEm2: tilem_z80_run_time)
        if ( (res = (exiting ? m_calc->run_cc(1) : m_calc->run_us(TIMEOUT))) )
        {
            qDebug() << "CalcThread: emulator stopped with result:" << res;
            if ( res & 0x01 ) // TILEM_STOP_BREAKPOINT
            {
                qDebug() << "CalcThread: breakpoint hit";
            }
            break;
        }

        // Desktop TilEm2 adaptive timing algorithm (emucore.c:622-654)
        if (m_limitSpeed) {
            // Track ideal completion time (desktop TilEm2: emu->timevalue)
            m_idealTime += TIMEOUT;

            // Get actual elapsed time (desktop TilEm2: g_timer_elapsed)
            qint64 realTime = m_timer.nsecsElapsed() / 1000;  // Convert nanoseconds to microseconds

            // Calculate delay needed (desktop TilEm2: delaytime = sub_us(emu->timevalue, tcur))
            qint64 delaytime = m_idealTime - realTime;

            if (delaytime > 0) {
                // Running faster than real-time - throttle (desktop TilEm2: g_usleep(delaytime))
                QThread::usleep(delaytime);

                if (loopCount % 1000 == 0) {
                    qDebug() << "CalcThread: ahead by" << delaytime << "us, sleeping";
                }
            }
            else if (delaytime < -RESYNC_THRESHOLD) {
                // Running slower than real-time by >100ms - resync (desktop TilEm2: emu->timevalue = tcur)
                m_idealTime = realTime;

                if (loopCount % 100 == 0) {
                    qDebug() << "CalcThread: behind by" << (-delaytime) << "us, resyncing";
                }
            }
            else if (delaytime < 0) {
                // Slightly behind - no sleep, try to catch up
                if (loopCount % 1000 == 0) {
                    qDebug() << "CalcThread: behind by" << (-delaytime) << "us, catching up (no sleep)";
                }
            }
        }

        if (loopCount++ % 1000 == 0) {
            qDebug() << "CalcThread: running, loop count:" << loopCount;
        }

        if ( exiting )
            break;
    }

    exiting = 0;

    qDebug() << "CalcThread: stop running";

    emit runningChanged(false);
}
