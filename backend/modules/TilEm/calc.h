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

#ifndef _CALC_H_
#define _CALC_H_

/*!
    \file calc.h
    \brief Definition of the Calc class
*/

#include "linkbuffer.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <tilem.h>

#include <QHash>
#include <QObject>
#include <QMutex>
#include <QByteArray>
#include <QReadWriteLock>
#include <QStringList>

class QScriptEngine;
class CalcLink;
class CalcThread;
class QMimeData;

class Calc : public QObject
{
    friend class CalcLink;
    friend class CalcDebugger;

    friend const char *tilem_gettext(const char *msg);

    friend void tilem_message(TilemCalc* calc, const char* msg, ...);
    friend void tilem_warning(TilemCalc* calc, const char* msg, ...);
    friend void tilem_internal(TilemCalc* calc, const char* msg, ...);

    Q_OBJECT
        Q_PROPERTY(QString file READ romFile WRITE load NOTIFY fileChanged)
        Q_PROPERTY(QString name READ name WRITE load NOTIFY nameChanged)
        Q_PROPERTY(QString modelName READ modelName NOTIFY modelNameChanged)
        Q_PROPERTY(QString modelDescription READ modelDescription NOTIFY modelDescriptionChanged)

    public:
        enum LogLevel
        {
            Message,
            Warning,
            Internal
        };

        typedef TilemZ80BreakpointFunc BreakCallback;

        Calc(QObject *p = 0);
        ~Calc();

        Q_INVOKABLE void pressKey(int keycode);
        Q_INVOKABLE void releaseKey(int keycode);
        // NOV 11 2025: Removed injectTimerInterrupt() - dead code (added Oct 14, disabled Oct 14)
        Q_INVOKABLE QStringList guessRomType(QString url);

        CalcLink* link() const;

        bool isValid();

        bool isPaused() const;
        bool isRunning() const;
        bool isAsleep() const;
        bool getPowerOnHalt() const;

        QString name() const;

        int model() const;
        QString modelName() const;
        QString modelDescription() const;

        QString romFile() const;

        void sendFile();

        bool lcdUpdate();
        Q_INVOKABLE int lcdWidth() const;
        Q_INVOKABLE int lcdHeight() const;
        const unsigned int* lcdData() const;

        void resetLink();

        bool isBroadcasting() const;
        void setBroadcasting(bool y);

        bool isSending() const;
        bool isReceiving() const;

        bool isSupported(const QMimeData *md);

        uint32_t byteCount() const;

        char topByte();
        char getByte();
        void sendByte(char c);

        QByteArray getBytes(int n);
        int getBytes(int n, char *d);
        void sendBytes(const QByteArray& d);

        int breakpointCount() const;

        void addBreakpoint(BreakCallback cb);
        void removeBreakpoint(int n);

        int breakpointType(int n) const;
        void setBreakpointType(int n, int type);

        bool isBreakpointPhysical(int n) const;
        void setBreakpointPhysical(int n, bool y);

        dword breakpointStartAddress(int n) const;
        void setBreakpointStartAddress(int n, dword a);

        dword breakpointEndAddress(int n) const;
        void setBreakpointEndAddress(int n, dword a);

        dword breakpointMaskAddress(int n) const;
        void setBreakpointMaskAddress(int n, dword a);

    public slots:
        dword run_us(int usec);
        dword run_cc(int clock);

        void stop(int reason = 0);

        void setName(const QString& n);

        void step();
        void pause();
        void resume();
        void reset();

        void save();

        void load(const QString& file);
        void save(const QString& file);

    Q_SIGNALS:
        void paused();
        void resumed();

        void paused(bool isPaused);
        void beginLoad();
        void loaded();
        void fileChanged(QString file);
        void nameChanged(QString name);
        void modelNameChanged(QString model);
        void modelDescriptionChanged(QString modelDescription);

        void bytesAvailable();

        void breakpoint(int id);
        void log(const QString& message, int type, dword addr);

    private:
        void setModel();

        typedef dword (*emulator)(TilemCalc *c, int amount, int *remaining);
        dword run(int amount, emulator emu);

        QString m_romFile, m_name;

        QMutex m_run;
        TilemCalc *m_calc;

        QList<int> m_breakIds;

        unsigned char *m_lcd;
        unsigned int *m_lcd_comp;

        volatile bool m_load_lock, m_link_lock, m_broadcast;

        LinkBuffer m_input, m_output;

        static QHash<TilemCalc*, Calc*> m_table;

        CalcLink *m_link;
        CalcThread *m_thread;
};

#endif
