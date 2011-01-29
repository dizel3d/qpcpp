//////////////////////////////////////////////////////////////////////////////
// Product:  Time Bomb Example with QEP (FSM support)
// Last Updated for Version: 4.1.01
// Date of the Last Update:  Nov 04, 2009
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qep_port.h"
#include "bsp.h"

enum Bomb4Signals {
    UP_SIG = Q_USER_SIG,
    DOWN_SIG,
    ARM_SIG,
    TICK_SIG
};

struct TickEvt : public QEvent {
    uint8_t fine_time;                              // the fine 1/10 s counter
};

class Bomb4 : public QFsm {
    uint8_t m_timeout;                     // number of seconds till explosion
    uint8_t m_code;               // currently entered code to disarm the bomb
    uint8_t m_defuse;                 // secret defuse code to disarm the bomb

public:
    Bomb4(uint8_t defuse)
        : QFsm((QStateHandler)&Bomb4::initial),
          m_defuse(defuse)
    {}

protected:
    static QState initial(Bomb4 *me, QEvent const *e);
    static QState setting(Bomb4 *me, QEvent const *e);
    static QState timing (Bomb4 *me, QEvent const *e);
};

//----------------------------------------------------------------------------
                                           // the initial value of the timeout
#define INIT_TIMEOUT   10

//............................................................................
QState Bomb4::initial(Bomb4 *me, QEvent const *) {
    me->m_timeout = INIT_TIMEOUT;
    return Q_TRAN(&Bomb4::setting);
}
//............................................................................
QState Bomb4::setting(Bomb4 *me, QEvent const *e) {
    switch (e->sig) {
        case UP_SIG: {
            if (me->m_timeout < 60) {
                ++me->m_timeout;
                BSP_display(me->m_timeout);
            }
            return Q_HANDLED();
        }
        case DOWN_SIG: {
            if (me->m_timeout > 1) {
                --me->m_timeout;
                BSP_display(me->m_timeout);
            }
            return Q_HANDLED();
        }
        case ARM_SIG: {
            return Q_TRAN(&Bomb4::timing);           // transition to "timing"
        }
    }
    return Q_IGNORED();
}
//............................................................................
QState Bomb4::timing(Bomb4 *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_code = 0;                           // clear the defuse code
            return Q_HANDLED();
        }
        case UP_SIG: {
            me->m_code <<= 1;
            me->m_code |= 1;
            return Q_HANDLED();
        }
        case DOWN_SIG: {
            me->m_code <<= 1;
            return Q_HANDLED();
        }
        case ARM_SIG: {
            if (me->m_code == me->m_defuse) {
                return Q_TRAN(&Bomb4::setting);     // transition to "setting"
            }
            return Q_HANDLED();
        }
        case TICK_SIG: {
            if (((TickEvt const *)e)->fine_time == 0) {
                --me->m_timeout;
                BSP_display(me->m_timeout);
                if (me->m_timeout == 0) {
                    BSP_boom();                            // destroy the bomb
                }
            }
            return Q_HANDLED();
        }
    }
    return Q_IGNORED();
}

// Test harness --------------------------------------------------------------
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

static Bomb4 bomb(0x0D);    // time bomb FSM, the secret defuse code, 1101 bin

//............................................................................
int main() {

    cout << "Time Bomb (QEP QFsm class)" << endl
         << "Press 'u'   for UP   event" << endl
         << "Press 'd'   for DOWN event" << endl
         << "Press 'a'   for ARM  event" << endl
         << "Press <Esc> to quit."       << endl;

    bomb.init();                                // take the initial transition

    static TickEvt tick_evt;
    tick_evt.sig = TICK_SIG;
    tick_evt.fine_time = 0;
    for (;;) {                                                   // event loop

        delay(100);                                            // 100 ms delay

        if (++tick_evt.fine_time == 10) {
            tick_evt.fine_time = 0;
        }
        cout.width(1);
        cout << "T(" << (int)tick_evt.fine_time << ')'
             << ((tick_evt.fine_time == 0) ? '\n' : ' ');

        bomb.dispatch(&tick_evt);                       // dispatch TICK event

        if (_kbhit()) {
            static QEvent const up_evt   = { UP_SIG,   0 };
            static QEvent const down_evt = { DOWN_SIG, 0 };
            static QEvent const arm_evt  = { ARM_SIG,  0 };
            QEvent const *e = (QEvent *)0;

            switch (_getch()) {
                case 'u': {                                        // UP event
                    cout << endl << "UP  : ";
                    e = &up_evt;                      // generate the UP event
                    break;
                }
                case 'd': {                                      // DOWN event
                    cout << endl << "DOWN: ";
                    e = &down_evt;                  // generate the DOWN event
                    break;
                }
                case 'a': {                                       // ARM event
                    cout << endl << "ARM : ";
                    e = &arm_evt;                    // generate the ARM event
                    break;
                }
                case '\33': {                                     // <Esc> key
                    cout << endl << "ESC : Bye! Bye!";
                    _exit(0);
                    break;
                }
            }
            if (e != (QEvent *)0) {               // keyboard event available?
                bomb.dispatch(e);                       // dispatch the event
            }
        }
    }

    return 0;
}
