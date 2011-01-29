//////////////////////////////////////////////////////////////////////////////
// Product: UI with State-Local Storage Example
// Last Updated for Version: 4.0.01
// Date of the Last Update:  Sep 19, 2008
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
#include "qp_port.h"                           // the port of the QP framework
#include "num_ent.h"
#include "ui.h"
#include "video.h"

// Local objects -------------------------------------------------------------
static UI_mem l_ui_mem;                 // instantiate the UI state machine...
                      // NOTE: this executes the ctor of the UI_top superstate

// Global objects ------------------------------------------------------------
QActive * const AO_UI = &l_ui_mem;     // "opaque" pointer to UI Active Object


// HSM definition ------------------------------------------------------------
QState UI_top::initial(UI_top *me, QEvent const * /* e */) {
                                                     // subscribe to events...
    me->subscribe(QUIT_SIG);
                                                     // initialize the history
    me->m_history = (QStateHandler)&UI_num_sd::handler;

                                    // send object dictionaries for UI objects
    QS_OBJ_DICTIONARY(&l_ui_mem);

                                // send function dictionaries for UI states...
    QS_FUN_DICTIONARY(&UI_top::handler);
    QS_FUN_DICTIONARY(&UI_top::final);
    QS_FUN_DICTIONARY(&UI_num::handler);
    QS_FUN_DICTIONARY(&UI_num_sd::handler);
    QS_FUN_DICTIONARY(&UI_num_lr::handler);
    QS_FUN_DICTIONARY(&UI_help::handler);

                     // send signal dictionaries for signals specific to UI...
    QS_SIG_DICTIONARY(C_SIG,         me);
    QS_SIG_DICTIONARY(CE_SIG,        me);
    QS_SIG_DICTIONARY(DIGIT_0_SIG,   me);
    QS_SIG_DICTIONARY(DIGIT_1_9_SIG, me);
    QS_SIG_DICTIONARY(POINT_SIG,     me);
    QS_SIG_DICTIONARY(NEG_SIG,       me);
    QS_SIG_DICTIONARY(ENTER_SIG,     me);
    QS_SIG_DICTIONARY(UP_SIG,        me);
    QS_SIG_DICTIONARY(DOWN_SIG,      me);
    QS_SIG_DICTIONARY(HELP_SIG,      me);

    return Q_TRAN(&UI_num::handler);
}
//............................................................................
QState UI_top::handler(UI_top *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            uint8_t c;
            Video::clearScreen(Video::BGND_BLACK);
            Video::clearRect( 0,  0, 80,  7, Video::BGND_LIGHT_GRAY);
            Video::clearRect( 0, 10, 80, 11, Video::BGND_LIGHT_GRAY);
            Video::clearRect( 0, 23, 80, 24, Video::BGND_LIGHT_GRAY);

            c = Video::FGND_BLUE;
            Video::printStrAt(10, 0, c, "  __");
            Video::printStrAt(10, 1, c, " /  |      _   _ -|-     _ _");
            Video::printStrAt(10, 2, c, " \\__| | |  _\\ | \\ | | | | \\ \\");
            Video::printStrAt(10, 3, c, "    | \\_/ |_| | | | \\_| | | |");
            Video::printStrAt(10, 4, c, "    |");
            c = Video::FGND_RED;
            Video::printStrAt(43, 0, c, "    _       __ ");
            Video::printStrAt(43, 1, c, "|  /_\\     |  \\  TM");
            Video::printStrAt(43, 2, c, "|  \\_   _  |__/ _");
            Video::printStrAt(43, 3, c, "|       _\\ |   |_");
            Video::printStrAt(43, 4, c, "|___   |_| |    _|");
            Video::printStrAt(10, 5, Video::FGND_BLUE,
                "_____________________________________________________");
            Video::printStrAt(10, 6, Video::FGND_RED,
                "i n n o v a t i n g   e m b e d d e d   s y s t e m s");
            Video::printStrAt(2,  8, Video::FGND_WHITE,
                "State-Local Storage Example");
            Video::printStrAt(36,  8, Video::FGND_WHITE, "QEP/C++");
            Video::printStrAt(45,  8, Video::FGND_YELLOW, QEP::getVersion());
            Video::printStrAt(55,  8, Video::FGND_WHITE, "QF/C++");
            Video::printStrAt(64,  8, Video::FGND_YELLOW, QF::getVersion());

            Video::printStrAt(10, 23, Video::FGND_BLUE,
              "* Copyright (c) Quantum Leaps, LLC * www.quantum-leaps.com *");
            Video::printStrAt(28, 24, Video::FGND_LIGHT_RED,
                "<< Press Esc to quit >>");

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            Video::clearScreen(Video::BGND_BLACK);      // clear the screen...
            return Q_HANDLED();
        }
        case QUIT_SIG: {
            return Q_TRAN(&UI_top::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState UI_top::final(UI_top *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            QF::stop();                                 // stop QF and cleanup
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

