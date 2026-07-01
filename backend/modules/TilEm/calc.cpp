/****************************************************************************
**
** Copyright (C) 2009,2010 Hugues Luc BRUANT aka fullmetalcoder
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

#include "calc.h"
#include "calclink.h"
#include "calcthread.h"

/*!
    \file calc.cp
    \brief Implementation of the Calc class
*/

#include <scancodes.h>

#include <QDir>
#include <QColor>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

#include <errno.h>

// ARM64 DEBUG (Oct 12, 2025 22:50 UTC): Qt debug bridge for TilEm core logging
// Provides tilem_qt_debug() to allow C code in tilem-core/emu to log to Qt/journalctl
// Verbose core logging is OFF by default. Set the env var TILEM_DEBUG=1 to
// re-enable the per-cycle HALT/timer diagnostics from tilem-core/emu.
extern "C" void tilem_qt_debug(const char* msg) {
	static const bool verbose = qEnvironmentVariableIsSet("TILEM_DEBUG");
	if (verbose)
		qDebug() << msg;
}

class RegisterDword
{
    public:
        RegisterDword()
        {
            qRegisterMetaType<dword>("dword");
        }
};

static RegisterDword rdw;

QHash<TilemCalc*, Calc*> Calc::m_table;

/*!
    \class Calc
    \brief Core class to manage calc emulation

    Abstracts away the use of libtilemcore and load/save of roms
*/

Calc::Calc(QObject *p)
 : QObject(p), m_calc(0), m_lcd(0), m_lcd_comp(0), m_link(NULL), m_thread(NULL)
{

}

Calc::~Calc()
{
    delete m_lcd_comp;
    delete m_lcd;
    if(m_thread)
        m_thread->stop();

    delete m_link;

    QMutexLocker lock(&m_run);

    if(m_calc) {
        m_table.remove(m_calc);

        // release memory
        tilem_calc_free(m_calc);
    }
}

CalcLink *Calc::link() const
{
    qDebug() << "Calc: link";
    return m_link;
}

/**
 * @brief Check if the calculator is valid
 *
 * @return true if valid
 */
bool Calc::isValid()
{
    if(m_calc)
        return true;
    return false;
}

/**
 * @brief Check if calculator is asleep (halted without poweronhalt)
 *
 * TilEm2-compatible implementation: returns true when Z80 is halted AND poweronhalt is NOT set.
 * This correctly indicates the calculator is in sleep mode.
 *
 * @return true if asleep
 */
bool Calc::isAsleep() const
{
    if (!m_calc)
        return false;
    return m_calc->z80.halted && !m_calc->poweronhalt;
}

/**
 * @brief Get poweronhalt state
 *
 * @return poweronhalt value
 */
bool Calc::getPowerOnHalt() const
{
    if (!m_calc)
        return false;
    return m_calc->poweronhalt != 0;
}

/**
 * @brief Whether the *thread* is paused
 *
 * @return
 */
bool Calc::isPaused() const
{
    qDebug() << "Calc: isPaused";
    if(m_thread)
        return !isRunning();
    return false;
}

/**
 * @brief Whether the *thread* is running
 *
 * @return
 */
bool Calc::isRunning() const
{
    qDebug() << "Calc: isRunning";
    if(m_thread)
        return m_thread->isRunning();
    return false;
}

/**
 * @brief Name of the current calculator
 *
 * @return the name
 */
QString Calc::name() const
{
    qDebug() << "Calc: name";
    return m_name;
}

/**
 * @brief Model of the current calculator
 *
 * @return
 */
int Calc::model() const
{
    qDebug() << "Calc: model";
    return m_calc ? m_calc->hw.model_id : 0;
}

/**
 * @brief Model name of the current calculator gotten from the rom
 *
 * @return
 */
QString Calc::modelName() const
{
    qDebug() << "Calc: modelName";
    return QString(m_calc->hw.name);
}

/**
 * @brief Model description of the current calculator gotten from the rom
 *
 * @return
 */
QString Calc::modelDescription() const
{
    qDebug() << "Calc: modelDescription";
    return QString(m_calc->hw.desc);
}

/**
 * @brief Rom file location currently loaded
 *
 * @return
 */
QString Calc::romFile() const
{
    qDebug() << "Calc: romFile";
    return m_romFile;
}

/**
 * @brief Set name of the calculator
 *
 * @param n
 */
void Calc::setName(const QString& n)
{
    qDebug() << "Calc: setName";
    if ( n == m_name )
        return;

    m_name = n;

    emit nameChanged(n);
}

/**
 * @brief Reset the link
 */
void Calc::resetLink()
{
    qDebug() << "Calc: resetLink";
    tilem_linkport_graylink_reset(m_calc);
}

bool Calc::isBroadcasting() const
{
    qDebug() << "Calc: isBroadcasting";
    return m_broadcast;
}

void Calc::setBroadcasting(bool y)
{
    qDebug() << "Calc: setBroadcasting";
    m_broadcast = y;
}

/**
 * @brief Whether the *calc* is writing data through the linkport
 *
 * @return 
 */
bool Calc::isSending() const
{
    qDebug() << "Calc: isSending";
    return m_output.count();
}

/**
 * @brief Whether the *calc* has data to read from the linkport
 *
 * @return 
 */
bool Calc::isReceiving() const
{
    qDebug() << "Calc: isReceiving";
    return m_input.count();
}

/**
 * @brief Whether the file is supported to be send 
 *
 * @param md
 *
 * @return 
 */
bool Calc::isSupported(const QMimeData *md)
{
    qDebug() << "Calc: isSupported";
    if ( md->hasUrls() )
    {
        QList<QUrl> l = md->urls();

        foreach ( QUrl url, l )
            if ( m_link->isSupportedFile(url.toLocalFile()) )
                return true;
    }

    return false;
}

/**
 * @brief Simulate keypress on the calc
 *
 * @param keycode Key to be pressed
 */
void Calc::pressKey(int keycode)
{
    qDebug() << "*** CALC::PRESSKEY CALLED! Keycode:" << keycode << "Valid:" << isValid();
    if(isValid()) {
        // Serialize keypad access against the emulator thread (run() holds m_run).
        // Without this lock, modifying m_calc->keypad while the Z80 core runs on
        // the CalcThread races the calc state and corrupts it on ARM64 -> freeze.
        QMutexLocker lock(&m_run);
        tilem_keypad_press_key(m_calc, keycode);
        qDebug() << "*** tilem_keypad_press_key() completed for keycode" << keycode;
    } else {
        qDebug() << "*** WARNING: Calc not valid, ignoring pressKey!";
    }
}

/**
 * @brief Simulate keyresease on the calc
 *
 * @param keycode Key to be released
 */
void Calc::releaseKey(int keycode)
{
    qDebug() << "Calc: release " << keycode;
    if(isValid()) {
        // Same reason as pressKey(): serialize against the emulator thread.
        QMutexLocker lock(&m_run);
        tilem_keypad_release_key(m_calc, keycode);
    }
}

// NOV 11 2025: Removed injectTimerInterrupt() function - DEAD CODE
// - Added Oct 14, 2025 as workaround for ARM64 interrupt storm
// - Disabled Oct 14, 2025 when hardware Timer1 found to work correctly
// - Never used after Oct 14 (see CalcPage.qml timer is disabled)
// - Function took 54 lines, provided no value
// - Hardware timers now properly configured (40-second period prevents storm)

QStringList Calc::guessRomType(QString url)
{
    FILE *romfile = fopen(qPrintable(url), "rb");

    if ( !romfile )
    {
        qWarning(qPrintable(tr("Unable to load ROM file \"%s\": %s")),
             qPrintable(url), strerror(errno));
        return QStringList();
    }
    char rom_type = tilem_guess_rom_type(romfile);

    QStringList options;
    int nmodels, selected;
    const TilemHardware** models;

    tilem_get_supported_hardware(&models, &nmodels);

    for ( int i = 0; i < nmodels; ++i )
    {
        options << QString::fromLatin1(models[i]->desc);

        if ( models[i]->model_id == rom_type )
            selected = i;
    }

    return options;
}

/**
 * @brief step the calc
 */
void Calc::step()
{
    qDebug() << "Calc: step";
    if(m_thread)
        m_thread->step();
}

/**
 * @brief Pause the calc
 */
void Calc::pause()
{
    qDebug() << "Calc: pauze";
    if ( m_thread && isRunning() )
        m_thread->stop();
}

/**
 * @brief Resume the calc
 */
void Calc::resume()
{
    qDebug() << "Calc: resume";
    if ( m_thread && !isRunning() )
        m_thread->start();
}

/**
 * @brief Save the calc
 */
void Calc::save()
{
    qDebug() << "Calc: save";
}

/**
 * @brief Load a rom file on the calc
 *
 * @param file The rom file to be loaded
 */
void Calc::load(const QString &file)
{
    qDebug() << "Calc: load a file" << file;
    /// 1) stop/cleanup phase
    emit beginLoad();

    if ( m_thread )
        m_thread->stop();

    // 2) load phase
    QMutexLocker lock(&m_run);

    QFileInfo info(file);

    FILE *romfile, *savefile = 0;

//    QString savefilename = QDir(info.path()).filePath(info.completeBaseName() + ".sav");

    romfile = fopen(qPrintable(file), "rb");

    if ( !romfile )
    {
        qWarning(qPrintable(tr("Unable to load ROM file \"%s\": %s")),
             qPrintable(file), strerror(errno));
        return;
    } else {
        m_load_lock = true;
        //qDebug("successfully opened %s", qPrintable(file));
    }

//    savefile = fopen(qPrintable(savefilename), "rt");

    if ( m_calc )
    {
        //qDebug("cleanin up previous state.");
        m_table.remove(m_calc);

        tilem_calc_free(m_calc);
        m_calc = 0;

        delete m_lcd_comp;
        m_lcd_comp = 0;

        delete m_lcd;
        m_lcd = 0;
    }

    m_romFile = file;

    char rom_type = tilem_guess_rom_type(romfile);

    QStringList options;
    int nmodels, selected = -1;
    const TilemHardware** models;

    tilem_get_supported_hardware(&models, &nmodels);

    for ( int i = 0; i < nmodels; ++i )
    {
        options << QString::fromLatin1(models[i]->desc);

        if ( models[i]->model_id == rom_type )
            selected = i;
    }

//    QString t =
//    QInputDialog::getItem(
//                        0,
//                        tr("Select ROM type"),
//                        tr("Select ROM type (keep default if unsure)"),
//                        options,
//                        selected,
//                        false
//                    );

//    selected = options.indexOf(t);

    if ( selected != -1 )
        rom_type = models[selected]->model_id;

    //qDebug("%c", rom_type);

    m_calc = tilem_calc_new(rom_type);
    m_table[m_calc] = this;

    // savefile is NULL
    tilem_calc_load_state(m_calc, romfile, savefile);

    // some link emulation magic...
    m_calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
    m_calc->z80.stop_mask &= ~(TILEM_STOP_LINK_READ_BYTE | TILEM_STOP_LINK_WRITE_BYTE | TILEM_STOP_LINK_ERROR);

    m_broadcast = true;
    m_link_lock = false;

    // instant LCD state and "composite" LCD state (grayscale is a bitch...)
    m_lcd = new unsigned char[m_calc->hw.lcdwidth * m_calc->hw.lcdheight / 8];
    m_lcd_comp = new unsigned int[m_calc->hw.lcdwidth * m_calc->hw.lcdheight];

    m_calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
    m_calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;

    fclose(romfile);

//    if ( savefile )
//        fclose(savefile);

    m_load_lock = false;

    emit fileChanged(romFile());

    if ( !m_link )
    {
        m_link = new CalcLink(this, this);

//        connect(m_link, SIGNAL( externalLinkGrabbed(bool) ), this, SIGNAL( externalLinkGrabbed(bool) ));
    } else
        m_link->setCalc(this);

    if ( !m_thread )
    {
        m_thread = new CalcThread(this, this);
        connect(m_thread, SIGNAL( started() ), this, SIGNAL( resumed() ) );
        connect(m_thread, SIGNAL( finished() ), this, SIGNAL( paused() ) );
        connect(m_thread, SIGNAL( runningChanged(bool) ), this, SIGNAL( paused(bool) ) );
    }

    // Release m_run before the restart phase. emit loaded() below runs its
    // connected slots synchronously on this thread, and CalcScreen::fileLoaded()
    // forces an immediate updateLCD() -> lcdUpdate(), which now locks m_run.
    // Holding it across the emit would self-deadlock (m_run is non-recursive).
    // The emulator thread started below acquires m_run itself, so it must be free.
    lock.unlock();

    /// 3) restart phase

    // launch emulator thread
    m_thread->start();

    // start LCD update timer
    emit loaded();

    setModel();
}

/**
 * @brief Save the current state
 *
 * @param file File to be saved to
 */
void Calc::save(const QString &file)
{
    qDebug() << "Calc: save to file";
    QMutexLocker lock(&m_run);

    QFileInfo info(file);

    FILE *romfile, *savefile;

    QString savefilename = QDir(info.path()).filePath(info.completeBaseName() + ".sav");

    if ( !(m_calc->hw.flags & TILEM_CALC_HAS_FLASH) )
    {
        romfile = NULL;
    } else if ( !(romfile = fopen(qPrintable(file), "wb")) ) {
        qWarning(qPrintable(tr("Unable to save ROM file \"%s\": %s")),
             qPrintable(file), strerror(errno));
    }

    if ( !(savefile = fopen(qPrintable(savefilename), "wt")) )
    {
        qWarning(qPrintable(tr("Unable to save state file \"%s\": %s")),
             qPrintable(savefilename), strerror(errno));
    }

    if ( romfile || savefile )
    {
        // save state
        tilem_calc_save_state(m_calc, romfile, savefile);
    }

    if ( romfile )
        fclose(romfile);

    if ( savefile )
        fclose(savefile);
}

/*!
    \return amount of data written to the linkport by the *calc* that can be retrieved using getByte()
*/
uint32_t Calc::byteCount() const
{
    qDebug() << "Calc: byteCount";
    return m_output.count();
}

/*!
    \brief get a byte from the buffer in which calc linkport writes are stored but does not remove it from the buffer
*/
char Calc::topByte()
{
    qDebug() << "Calc: topByte";
    if ( m_output.isEmpty() )
        return 0;

    char c = m_output.at(0);

    return c;
}

/*!
    \brief get a byte from the buffer in which calc linkport writes are stored
*/
char Calc::getByte()
{
    qDebug() << "Calc: getByte";
    if ( m_output.isEmpty() )
        return 0;

    char c = m_output.at(0);

    m_output.remove(1);

    return c;
}

/*!
    \brief send a byte to the calc
*/
void Calc::sendByte(char c)
{
    qDebug() << "Calc: sendByte";
    m_input += c;
}

/*!
    \brief get up to n bytes from the buffer in which calc linkport writes are stored
*/
QByteArray Calc::getBytes(int n)
{
    qDebug() << "Calc: getBytes";
    if ( n < 0 || m_output.isEmpty() )
        return QByteArray();

    n = qMin(n, (int)m_output.count());

    return m_output.take(n);
}

/*!
    \brief get up to n bytes from the buffer in which calc linkport writes are stored
*/
int Calc::getBytes(int n, char *d)
{
    qDebug() << "Calc: getBytes";
    if ( n < 0 || m_output.isEmpty() )
        return 0;

    n = qMin(n, (int)m_output.count());

    m_output.take(n, d);

    return n;
}

/*!
    \brief send bytes to the calc
*/
void Calc::sendBytes(const QByteArray& d)
{
    qDebug() << "Calc: sendBytes";
    m_input += d;
}

int Calc::breakpointCount() const
{
    qDebug() << "Calc: breakpointCount";
    return m_breakIds.count();
}

void Calc::addBreakpoint(BreakCallback cb)
{
    qDebug() << "Calc: addBreakpoint";
    m_breakIds << tilem_z80_add_breakpoint(m_calc, TILEM_BREAK_MEM_EXEC, 0, 0, -1, cb, 0);
}

void Calc::removeBreakpoint(int n)
{
    qDebug() << "Calc: removeBreakpoint";
    if ( n >= 0 && n < m_breakIds.count() )
        tilem_z80_remove_breakpoint(m_calc, m_breakIds.takeAt(n));
}

int Calc::breakpointType(int n) const
{
    qDebug() << "Calc: breakpointType";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    return tilem_z80_get_breakpoint_type(m_calc, id) & TILEM_BREAK_TYPE_MASK;
}

void Calc::setBreakpointType(int n, int type)
{
    qDebug() << "Calc: setBreakpointType";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    int oldtype = tilem_z80_get_breakpoint_type(m_calc, id);
    type |= oldtype & ~TILEM_BREAK_TYPE_MASK;
    tilem_z80_set_breakpoint_type(m_calc, id, type);
}

bool Calc::isBreakpointPhysical(int n) const
{
    qDebug() << "Calc: isBreakpointPhysical";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    return tilem_z80_get_breakpoint_type(m_calc, id) & TILEM_BREAK_PHYSICAL;
}

void Calc::setBreakpointPhysical(int n, bool y)
{
    qDebug() << "Calc: setBreakpointPhysical";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    int type = tilem_z80_get_breakpoint_type(m_calc, id);

    if (y)
        type |= TILEM_BREAK_PHYSICAL;
    else
        type &= ~TILEM_BREAK_PHYSICAL;

    tilem_z80_set_breakpoint_type(m_calc, id, type);
}

dword Calc::breakpointStartAddress(int n) const
{
    qDebug() << "Calc: breakpointStartAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    return tilem_z80_get_breakpoint_address_start(m_calc, id);
}

void Calc::setBreakpointStartAddress(int n, dword a)
{
    qDebug() << "Calc: setBreakpointStartAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    tilem_z80_set_breakpoint_address_start(m_calc, id, a);
}

dword Calc::breakpointEndAddress(int n) const
{
    qDebug() << "Calc: breakpointEndAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    return tilem_z80_get_breakpoint_address_end(m_calc, id);
}

void Calc::setBreakpointEndAddress(int n, dword a)
{
    qDebug() << "Calc: setBreakpointEndAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    tilem_z80_set_breakpoint_address_end(m_calc, id, a);
}

dword Calc::breakpointMaskAddress(int n) const
{
    qDebug() << "Calc: breakpointMaskAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    return tilem_z80_get_breakpoint_address_mask(m_calc, id);
}

void Calc::setBreakpointMaskAddress(int n, dword a)
{
    qDebug() << "Calc: setBreakpointMaskAddress";
    int id = n >= 0 && n < m_breakIds.count() ? m_breakIds.at(n) : 0;

    tilem_z80_set_breakpoint_address_mask(m_calc, id, a);
}

/*!
    \brief Rest calc (simulate battery pull)
*/
void Calc::reset()
{
    qDebug() << "Calc: reset";
    QMutexLocker lock(&m_run);

    byte keys[8];

    memcpy(keys, m_calc->keypad.keysdown, 8 * sizeof(byte));

    tilem_calc_reset(m_calc);

    memcpy(m_calc->keypad.keysdown, keys, 8 * sizeof(byte));
}

dword Calc::run_us(int usec)
{
    return run(usec, tilem_z80_run_time);
}

dword Calc::run_cc(int clock)
{
    return run(clock, tilem_z80_run);
}

dword Calc::run(int amount, emulator emu)
{
    QMutexLocker lock(&m_run);

    if ( !m_calc )
        return -1;

    int remaining = amount;

    do
    {
        // try to forward data written into input buffer to link port

        if ( !m_link_lock )
        {
            if ( m_input.count() )
            {
                m_calc->z80.stop_reason = 0;

                if ( !tilem_linkport_graylink_send_byte(m_calc, m_input.at(0)) )
                {
                    #ifdef TILEM_QT_LINK_DEBUG
                    printf("@> %02x", static_cast<unsigned char>(m_input.at(0)));
                    #endif

                    m_input.remove(1);

                    if ( !(m_calc->z80.stop_reason & TILEM_STOP_LINK_WRITE_BYTE) )
                    {
                        m_link_lock = true;
                    } else {
                        // here's the trick to speed things up : batch processing whenever possible
                        while ( m_input.count() && (m_calc->z80.stop_reason & TILEM_STOP_LINK_WRITE_BYTE) )
                        {
                            m_calc->z80.stop_reason = 0;

                            if ( !tilem_linkport_graylink_send_byte(m_calc, m_input.at(0)) )
                            {
                                #ifdef TILEM_QT_LINK_DEBUG
                                printf(" %02x", static_cast<unsigned char>(m_input.at(0)));
                                #endif
                                m_input.remove(1);
                            }
                        }
                    }

                    #ifdef TILEM_QT_LINK_DEBUG
                    printf("\n");
                    fflush(stdout);
                    #endif
                }
            } else {
                int b = tilem_linkport_graylink_get_byte(m_calc);

                if ( b != -1 )
                {
                    // one byte successfully read yay!

                    m_output += b;

                    #ifdef TILEM_QT_LINK_DEBUG
                    qDebug("@< %02x [%i] [0x%x]", static_cast<unsigned char>(b), m_output.count(), this);
                    #endif

                    if ( m_broadcast )
                        emit bytesAvailable();
                }
            }
        }

        dword res = emu(m_calc, remaining, &remaining);

        /*
            some link emulation magic : seamlessly transfer
            data from buffers to the calc using a virtual
            graylink to allow asynchronous transfers
        */
        if ( res & TILEM_STOP_LINK_WRITE_BYTE )
        {
            m_link_lock = false;
            remaining = qMax(1, remaining);
        }

        if ( res & TILEM_STOP_LINK_ERROR )
        {
            qWarning("@link error.");
            tilem_linkport_graylink_reset(m_calc);
            break;
        }

//         if ( res & TILEM_STOP_INVALID_INST )
//         {
//             break;
//         }
//
//         if ( res & TILEM_STOP_UNDOCUMENTED_INST )
//         {
//             break;
//         }
//
        if ( res & TILEM_STOP_BREAKPOINT )
        {
            emit breakpoint(m_calc->z80.stop_breakpoint);
            break;
        }
    } while ( remaining > 0 );

    return m_calc->z80.stop_reason;
}

/**
 * @brief Stop the emulator
 *
 * @param reason
 */
void Calc::stop(int reason)
{
    qDebug() << "Calc: stop";
    // QMutexLocker lock(&m_run);

    if ( !m_calc )
        return;

    tilem_z80_stop(m_calc, reason);
}

/**
 * @brief Update the screen of the calculator
 *
 * @return 
 */
bool Calc::lcdUpdate()
{
    // Runs on the GUI thread every ~10 ms. It reads m_calc (lcd/z80 state) and
    // calls hw.get_lcd(), which walks calc memory. The emulator thread mutates
    // exactly that state under m_run, so read it under the same lock to avoid a
    // data race with the running Z80 core.
    QMutexLocker lock(&m_run);

    if ( m_load_lock || !m_calc )
        return false;

    // low : black, high : white
    unsigned int low, high;
    const int cc = int(m_calc->lcd.contrast);
    const unsigned int end = m_calc->hw.lcdheight * m_calc->hw.lcdwidth;

    // contrast determination
    // CRITICAL FIX (Oct 13, 2025): Match upstream TilEm2 monolcd.c:79 logic
    // Upstream: Screen OFF when (!lcd.active OR (halted AND !poweronhalt))
    // Inverted: Screen ON when (lcd.active AND (!halted OR poweronhalt))
    // This correctly shows LCD during sleep mode (halted=1, poweronhalt=1)

    if ( m_calc->lcd.active && (!m_calc->z80.halted || m_calc->poweronhalt) )
    {
        // update "plain" LCD data
        (*m_calc->hw.get_lcd)(m_calc, m_lcd);

        /*
            LCD behaves roughly as follows :

            0->31 : high is white, low goes from white to black
            32->63 : low is black, high goes from white to black
        */
        const int c = 63 - cc;
        //low = qMin(31, c) / 8  + qMax(0, c - 31) * 4;
        //high = qMax(31, c) * 4 + qMin(0, c - 31) / 8;

        low = 0x00;
        high = 0xff;

//         if ( c < 32 ) {
//             low = 0;
//             high = c * 8;
//         } else {
//             low = (c - 32) * 8;
//             high = 255;
//         }
    } else {
        low = high = 0xff;
    }

    // update "composite" LCD data
    bool changed = false;

    for ( unsigned int idx = 0; idx < end; ++idx )
    {
        unsigned int v = m_lcd[idx >> 3] & (0x80 >> (idx & 7)) ? low : high;

        // blending for grayscale
        unsigned int g = v + ((qRed(m_lcd_comp[idx]) - v) * 7) / 8;

        v = qRgb(g, g, g);

        if ( v != m_lcd_comp[idx] )
        {
            changed = true;
            m_lcd_comp[idx] = v;
        }
    }

    return changed;
}

/**
 * @brief Width of the lcd of the emulator
 *
 * @return 
 */
int Calc::lcdWidth() const
{
    return m_calc ? m_calc->hw.lcdwidth : 0;
}

/**
 * @brief Height of the lcd of the emulator
 *
 * @return 
 */
int Calc::lcdHeight() const
{
    return m_calc ? m_calc->hw.lcdheight : 0;
}

void Calc::setModel()
{
    qDebug() << "QmlCalc: setModel";
    emit modelNameChanged(modelName());
    emit modelDescriptionChanged(modelDescription());
}

const unsigned int* Calc::lcdData() const
{
    return m_lcd_comp;
}

/*
    Implementation of libtilemcore memory allocation and debug output routines
*/

extern "C" {

void* tilem_realloc(void* p, size_t s)
{
    if (s) {
        if (p) {
            p = realloc(p, s);
        }
        else {
            p = malloc(s);
        }

        if (!p) {
            fprintf(stderr, "out of memory (need %lu bytes)\n",
                (unsigned long) s);
            abort();
        }
    }
    else if (p) {
        free(p);
        p = 0;
    }

    return p;
}

void tilem_free(void* p)
{
    tilem_realloc(p, 0);
}

void* tilem_malloc(size_t s)
{
    return tilem_realloc(0, s);
}

void* tilem_try_malloc(size_t s)
{
    return malloc(s);
}

void* tilem_malloc0(size_t s)
{
    void* p = calloc(s, 1);

    if (!p) {
        fprintf(stderr, "out of memory (need %lu bytes)\n",
            (unsigned long) s);
        abort();
    }

    return p;
}

void* tilem_try_malloc0(size_t s)
{
    return calloc(s, 1);
}

void* tilem_malloc_atomic(size_t s)
{
    return tilem_malloc(s);
}

void* tilem_try_malloc_atomic(size_t s)
{
    return malloc(s);
}
}

/* Logging */

extern "C" const char* tilem_gettext(const char *msg)
{
    /*
    char *result = new char[strlen(msg)];
    strcpy(result, msg);

    fprintf(stderr, "GETTEXT: ");
    fprintf(stderr, "%s", msg);
    fputc('\n', stderr);
    */

    return "";
}

extern "C" void tilem_message(TilemCalc* calc, const char* msg, ...)
{
    /*
    va_list ap;
    va_start(ap, msg);

    Calc *c = Calc::m_table.value(calc, 0);

    if ( false )
    {
        emit c->log(QString().vsprintf(msg, ap), Calc::Message, c->m_calc->z80.r.pc.w.l);
    } else {
        fprintf(stderr, "x%c: ", calc->hw.model_id);
        //vfprintf(stderr, msg, ap);
        fputc('\n', stderr);
    }

    va_end(ap);
    */
}

extern "C" void tilem_warning(TilemCalc* calc, const char* msg, ...)
{
    /*
    va_list ap;
    va_start(ap, msg);

    Calc *c = Calc::m_table.value(calc, 0);

    if ( c )
    {
        emit c->log(QString().vsprintf(msg, ap), Calc::Warning, c->m_calc->z80.r.pc.w.l);
    } else {
        fprintf(stderr, "x%c: WARNING: ", calc->hw.model_id);
        //vfprintf(stderr, msg, ap);
        fputc('\n', stderr);
    }

    va_end(ap);
    */
}

extern "C" void tilem_internal(TilemCalc* calc, const char* msg, ...)
{
    /*
    va_list ap;
    va_start(ap, msg);

    Calc *c = Calc::m_table.value(calc, 0);

    if ( c )
    {
        emit c->log(QString().vsprintf(msg, ap), Calc::Internal, c->m_calc->z80.r.pc.w.l);
    } else {
        fprintf(stderr, "x%c: INTERNAL ERROR: ", calc->hw.model_id);
        //vfprintf(stderr, msg, ap);
        fputc('\n', stderr);
    }

    va_end(ap);
    */
}
