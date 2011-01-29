//////////////////////////////////////////////////////////////////////////////
// Product: QHsmTst Example
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
#include "qhsmtst.h"

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QSignal sig);

//............................................................................
int main(int argc, char *argv[ ]) {
    if (argc > 1) {                                     // file name provided?
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) {                      // interactive version?
        l_outFile = stdout;

        printf("QHsmTst example, built on %s at %s\n"
               "QEP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QEP::getVersion());

        the_hsm->init();          // trigger the initial tran. in the test HSM

        for (;;) {                                               // event loop
            printf("\n>");

            int c;
            c = _getche();       // get a character from the console with echo
            printf(": ");

            QEvent e;
            if ('a' <= c && c <= 'i') {                           // in range?
                e.sig = (QSignal)(c - 'a' + A_SIG);
            }
            else if ('A' <= c && c <= 'I') {                      // in range?
                e.sig = (QSignal)(c - 'A' + A_SIG);
            }
            else if (c == '\33') {                             // the ESC key?
                e.sig = TERMINATE_SIG;       // terminate the interactive test
            }
            else {
                e.sig = IGNORE_SIG;
            }

            the_hsm->dispatch(&e);                       // dispatch the event
        }
    }
    else {                                                    // batch version
        printf("QHsmTst, output saved to %s\n", argv[1]);
        fprintf(l_outFile,
                "QHsmTst example, QEP %s\n", QEP::getVersion());

        the_hsm->init();          // trigger the initial tran. in the test HSM

                                                        // dynamic transitions
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(I_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(G_SIG);
        dispatch(H_SIG);
        dispatch(H_SIG);
        dispatch(C_SIG);
        dispatch(G_SIG);
        dispatch(C_SIG);
        dispatch(C_SIG);

        fclose(l_outFile);
    }

    return 0;
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    _exit(-1);
}
//............................................................................
void BSP_display(char const *msg) {
    fprintf(l_outFile, msg);
}
//............................................................................
void BSP_exit(void) {
    printf("Bye, Bye!");
    _exit(0);
}
//............................................................................
static void dispatch(QSignal sig) {
    QEvent e;
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    e.sig = sig;
    fprintf(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    the_hsm->dispatch(&e);                               // dispatch the event
}

