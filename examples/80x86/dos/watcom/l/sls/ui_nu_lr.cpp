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

#include <stdio.h>
#include <math.h>

#include <new.h>                                          // for placement new

class UI_num_lr_xy : public UI_num_lr {                         // x/y entries
public:
    static QState x_entry(UI_num_lr_xy *me, QEvent const *e);
    static QState y_entry(UI_num_lr_xy *me, QEvent const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num_lr_xy) < sizeof(UI_mem));

// Local objects -------------------------------------------------------------
static QEvent const l_clear_evt = { C_SIG, 0 };

#define NUM_ENTRY1_X     3
#define NUM_ENTRY1_Y     13

#define NUM_ENTRY2_X     (NUM_ENTRY1_X + NUM_STR_WIDTH + 4)
#define NUM_ENTRY2_Y     13

#define NUM_ENTRY_COLOR  (Video::FGND_YELLOW)

//............................................................................
QState UI_num_lr::handler(UI_num_lr *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            uint8_t c;
                                        // instantiate the state-local objects
            me->m_n       = 0.0;
            me->m_xsum    = 0.0;
            me->m_xsum_sq = 0.0;
            me->m_ysum    = 0.0;
            me->m_ysum_sq = 0.0;
            me->m_xysum   = 0.0;

            Video::printStrAt(2, 10, Video::FGND_BLACK,
                "Screen 2: Linear Regression      ");
            Video::clearRect( 0, 11, 35, 23, Video::BGND_BLUE);
            Video::clearRect(35, 11, 80, 23, Video::BGND_BLACK);

            c = Video::FGND_LIGHT_GRAY;
            Video::printStrAt(36, 12, c,
                "Press '-'        to enter a negative number");
            Video::printStrAt(36, 13, c,
                "Press '0' .. '9' to enter a digit");
            Video::printStrAt(36, 14, c,
                "Press '.'        to enter the decimal point");
            Video::printStrAt(36, 15, c,
                "Press <Enter>    to move from x to y");
            Video::printStrAt(43, 16, c,
                       "and again to enter the (x, y) sample");
            Video::printStrAt(36, 17, c,
                "Press 'e' or 'E' to Cancel last entry");
            Video::printStrAt(36, 18, c,
                "Press 'c' or 'C' to Cancel the data set");

            c = Video::FGND_WHITE;
            Video::printStrAt(36, 20, c,
                "Press UP-arrow   for previous screen");
            Video::printStrAt(36, 21, c,
                "Press DOWN-arrow for next screen");
            Video::printStrAt(36, 22, c,
                "Press F1         for help");

            Video::clearRect(NUM_ENTRY1_X, NUM_ENTRY1_Y,
                NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 1,
                Video::BGND_BLACK);

            Video::clearRect(NUM_ENTRY2_X, NUM_ENTRY2_Y,
                NUM_ENTRY2_X + NUM_STR_WIDTH, NUM_ENTRY2_Y + 1,
                Video::BGND_BLACK);

            me->m_num_entry.config(NUM_ENTRY1_X, NUM_ENTRY1_Y,
                NUM_ENTRY_COLOR);
            me->m_num_entry.dispatch(&l_clear_evt);

            me->m_num_entry.config(NUM_ENTRY2_X, NUM_ENTRY2_Y,
                NUM_ENTRY_COLOR);
            me->m_num_entry.dispatch(&l_clear_evt);

            c = Video::FGND_WHITE;                                   // labels
            Video::printStrAt(NUM_ENTRY1_X - 2, NUM_ENTRY1_Y, c, "x");
            Video::printStrAt(NUM_ENTRY2_X - 2, NUM_ENTRY2_Y, c, "y");

            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 3,
                c, "y = a*x + b");

            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 5,
                c, "n =");

            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 6,
                c, "a =");
            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 7,
                c, "b =");

            c = Video::FGND_YELLOW;                                  // values
            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                NUM_ENTRY1_Y + 5, c, "0           ");
            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                NUM_ENTRY1_Y + 6, c, "N/A         ");
            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                NUM_ENTRY1_Y + 7, c, "N/A         ");

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
                                                // store this state in history
            me->m_history = (QStateHandler)&UI_num_lr::handler;
                                         // destroy the state-local objects...
            // noting to destroy
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
                                // send function dictionaries for UI states...
            QS_FUN_DICTIONARY(&UI_num_lr_xy::x_entry);
            QS_FUN_DICTIONARY(&UI_num_lr_xy::y_entry);

            return Q_TRAN(&UI_num_lr_xy::x_entry);
        }
        case NEG_SIG:
        case DIGIT_0_SIG:
        case DIGIT_1_9_SIG:
        case POINT_SIG: {
            me->m_num_entry.dispatch(e);        // dispatch to the number comp
            return Q_HANDLED();
        }
        case CE_SIG: {
            me->m_num_entry.dispatch(&l_clear_evt);
            return Q_HANDLED();
        }
        case C_SIG: {
            return Q_TRAN(&UI_num_lr::handler);          // transition-to-self
        }
        case UP_SIG: {
            return Q_TRAN(&UI_num_sd::handler);             // previous Screen
        }
        case DOWN_SIG: {
            return Q_TRAN(&UI_num_sd::handler);                 // next Screen
        }
    }
    return Q_SUPER(&UI_num::handler);
}
//............................................................................
QState UI_num_lr_xy::x_entry(UI_num_lr_xy *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            Video::clearRect(NUM_ENTRY1_X, NUM_ENTRY1_Y,
                NUM_ENTRY1_X + NUM_STR_WIDTH, NUM_ENTRY1_Y + 1,
                Video::BGND_BLACK);
                                                // indicate the keyboard focus
            Video::drawRect(NUM_ENTRY1_X - 1, NUM_ENTRY1_Y - 1,
                NUM_ENTRY1_X + NUM_STR_WIDTH + 1, NUM_ENTRY1_Y + 2,
                Video::FGND_WHITE, 2);

            me->m_num_entry.config(NUM_ENTRY1_X, NUM_ENTRY1_Y,
                NUM_ENTRY_COLOR);
            me->m_num_entry.dispatch(&l_clear_evt);

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_x = me->m_num_entry.get();
                                                  // remove the keyboard focus
            Video::drawRect(NUM_ENTRY1_X - 1, NUM_ENTRY1_Y - 1,
                NUM_ENTRY1_X + NUM_STR_WIDTH + 1, NUM_ENTRY1_Y + 2,
                Video::FGND_WHITE, 0);
            return Q_HANDLED();
        }
        case ENTER_SIG: {
            return Q_TRAN(&UI_num_lr_xy::y_entry);
        }
    }
    return Q_SUPER(&UI_num_lr::handler);
}
//............................................................................
QState UI_num_lr_xy::y_entry(UI_num_lr_xy *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            Video::clearRect(NUM_ENTRY2_X, NUM_ENTRY2_Y,
                NUM_ENTRY2_X + NUM_STR_WIDTH, NUM_ENTRY2_Y + 1,
                Video::BGND_BLACK);
                                                // indicate the keyboard focus
            Video::drawRect(NUM_ENTRY2_X - 1, NUM_ENTRY2_Y - 1,
                NUM_ENTRY2_X + NUM_STR_WIDTH + 1, NUM_ENTRY2_Y + 2,
                Video::FGND_WHITE, 2);

            me->m_num_entry.config(NUM_ENTRY2_X, NUM_ENTRY2_Y,
                NUM_ENTRY_COLOR);
            me->m_num_entry.dispatch(&l_clear_evt);

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_num_entry.dispatch(&l_clear_evt);
                                                  // remove the keyboard focus
            Video::drawRect(NUM_ENTRY2_X - 1, NUM_ENTRY2_Y - 1,
                NUM_ENTRY2_X + NUM_STR_WIDTH + 1, NUM_ENTRY2_Y + 2,
                Video::FGND_WHITE, 0);
            return Q_HANDLED();
        }
        case CE_SIG: {
            return Q_TRAN(&UI_num_lr_xy::x_entry);
        }
        case ENTER_SIG: {
            double tmp = me->m_num_entry.get();                           // y
            char   buf[14];

            me->m_n       += 1.0;
            me->m_xsum    += me->m_x;
            me->m_ysum    += tmp;
            me->m_xsum_sq += me->m_x*me->m_x;
            me->m_ysum_sq += tmp*tmp;
            me->m_xysum   += me->m_x*tmp;

            sprintf(buf, "%-12.6g", me->m_n);                             // n
            Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                NUM_ENTRY1_Y + 5, Video::FGND_YELLOW, buf);

            tmp = me->m_xsum_sq - me->m_xsum * me->m_xsum / me->m_n;
            if ((tmp < -0.0000001) || (0.0000001 < tmp)) {
                double a = (me->m_xysum - me->m_xsum * me->m_ysum / me->m_n)
                           / tmp;
                double b = (me->m_ysum - a * me->m_xsum) / me->m_n;
                sprintf(buf, "%-12.6g", a);
                Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                    NUM_ENTRY1_Y + 6, Video::FGND_YELLOW, buf);
                sprintf(buf, "%-12.6g", b);
                Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                    NUM_ENTRY1_Y + 7, Video::FGND_YELLOW, buf);
            }
            else {
                Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                    NUM_ENTRY1_Y + 6, Video::FGND_YELLOW,  "N/A         ");
                Video::printStrAt(NUM_ENTRY1_X + NUM_STR_WIDTH + 5,
                    NUM_ENTRY1_Y + 7, Video::FGND_YELLOW,  "N/A         ");
            }

            return Q_TRAN(&UI_num_lr_xy::x_entry);
        }
    }
    return Q_SUPER(&UI_num_lr::handler);
}
