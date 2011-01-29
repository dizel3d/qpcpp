//////////////////////////////////////////////////////////////////////////////
// Product: Ultimate Hook state pattern example
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 07, 2008
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
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
#include "qassert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Q_DEFINE_THIS_FILE


class UltimateHook : public QHsm {
public:
    UltimateHook() : QHsm((QStateHandler)&UltimateHook::initial) {
    }

private:
    static QState initial (UltimateHook *me, QEvent const *e);
    static QState generic (UltimateHook *me, QEvent const *e);
    static QState specific(UltimateHook *me, QEvent const *e);
    static QState final   (UltimateHook *me, QEvent const *e);
};

enum UltimateHookSignals {
    A_SIG = Q_USER_SIG,
    B_SIG,
    C_SIG,
    D_SIG
};

//............................................................................
QState UltimateHook::initial(UltimateHook *me, QEvent const *) {
    return Q_TRAN(&UltimateHook::generic);
}
//............................................................................
QState UltimateHook::final(UltimateHook *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> final\n");
            printf("\nBye!Bye!\n");
            exit(0);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState UltimateHook::generic(UltimateHook *me, QEvent const *e) {
    switch (e->sig) {
        case Q_INIT_SIG: {
            printf("generic:init;");
            return Q_TRAN(&UltimateHook::specific);
        }
        case A_SIG: {
            printf("generic:A;");
            return Q_HANDLED();
        }
        case B_SIG: {
            printf("generic:B;");
            return Q_HANDLED();
        }
        case C_SIG: {
            printf("generic:C(reset);");
            return Q_TRAN(&UltimateHook::generic);
        }
        case D_SIG: {
            return Q_TRAN(&UltimateHook::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState UltimateHook::specific(UltimateHook *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("specific:entry;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            printf("specific:exit;");
            return 0;
        }
        case A_SIG: {
            printf("specific:A;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&UltimateHook::generic);
}
// test harness ==============================================================

#include <conio.h>

static UltimateHook l_test;

int main() {
    printf("Ultimate Hook pattern\nQEP version: %s\n"
           "Press 'a'..'c' to inject signals A..C\n"
           "Press 'd' or ESC to inject signal D and quit\n",
           QEP::getVersion());

    l_test.init(); // trigger the initial transition before dispatching events

    for (;;) {
        printf("\n");
        uint8_t c;
        c = (uint8_t)getch();           // read one character from the console
        printf("%c: ", c);

        QEvent e;
        switch (c) {
            case 'a':  e.sig = A_SIG;  break;
            case 'b':  e.sig = B_SIG;  break;
            case 'c':  e.sig = C_SIG;  break;
            case 'd':
            case 0x1B: e.sig = D_SIG;  break;                     // terminate
        }
        l_test.dispatch(&e);      // dispatch the event into the state machine
    }
    return 0;
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    _exit(-1);
}
