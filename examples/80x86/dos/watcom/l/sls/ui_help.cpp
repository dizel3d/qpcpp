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

#define HELP_X      3
#define HELP_Y      14
#define HELP_DX     28
#define HELP_DY     5

// Local objects -------------------------------------------------------------
static char const * const l_help_sd[] = {
    "Standard Deviation:         ",
    "Find the mean value and the ",
    " root-mean-square (RMS)     ",
    "deviation of n data samples ",
    "xi, where i = 1..n.         ",
    "Mean value <x> is calculated",
    "as follows:                 ",
    "<x> = Sum(xi)/n;            ",
    "Two RMS estimatators are    ",
    "provided:                   ",
    "sig(n) =                    ",
    "   sqrt(Sum(xi-<x>)**2 / n);",
    "sig(n-1) =                  ",
    "sqrt(Sum(xi-<x>)**2 / (n-1))"
};
static char const * const l_help_lr[] = {
    "Linear Regression:          ",
    "Fit the line y = a*x + b,   ",
    "based on n samples (xi, yi).",
    "The coefficients a and b are",
    "calculated as follows:      ",
    "a = Sxy / Sxx =             ",
    " (Sum(xi*yi)                ",
    "  -Sum(xi)*Sum(yi)/n)       ",
    " /(Sum(xi*xi)-(Sum(xi)**2/n;",
    "b = yave - a*xave =         ",
    " Sum(yi)/n - a*Sum(xi)/n;   "
};
static char const * const l_help_unknown[] = {
    "Unknown Screen Help:        ",
    "                            ",
    "                            ",
    "                            ",
    "                            "
};

static void printHelp(char const * const *txt) {
    uint8_t y;
    for (y = 0; y < HELP_DY; ++y) {
        Video::printStrAt(HELP_X, HELP_Y + y, Video::FGND_YELLOW, txt[y]);
    }
}

//............................................................................
QState UI_help::handler(UI_help *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                                        // instantiate the state-local objects
            Video::printStrAt(2, 10, Video::FGND_BLACK,
                "Screen 0: Help                   ");
            Video::clearRect( 0, 11, 35, 23, Video::BGND_BLUE);
            Video::clearRect(35, 11, 80, 23, Video::BGND_BLACK);

            Video::printStrAt(36, 12, Video::FGND_LIGHT_GRAY,
                "Press DOWN-Arrow to scroll down");
            Video::printStrAt(36, 13, Video::FGND_LIGHT_GRAY,
                "Press UP-Arrow   to scroll up");

            Video::printStrAt(36, 20, Video::FGND_WHITE,
                "Press F1         to return to last screen");

            Video::clearRect(HELP_X - 1, HELP_Y,
                HELP_X + HELP_DX + 1, HELP_Y + HELP_DY, Video::BGND_BLACK);
            Video::drawRect (HELP_X - 2, HELP_Y - 1,
                HELP_X + HELP_DX + 2, HELP_Y + HELP_DY + 1,
                Video::FGND_WHITE,2);

            if (me->m_history == (QStateHandler)&UI_num_sd::handler) {
                me->m_help_text = l_help_sd;
                me->m_help_len  = Q_DIM(l_help_sd);
            }
            else if (me->m_history == (QStateHandler)&UI_num_lr::handler) {
                me->m_help_text = l_help_lr;
                me->m_help_len  = Q_DIM(l_help_lr);
            }
            else {
                me->m_help_text = l_help_unknown;
                me->m_help_len  = Q_DIM(l_help_unknown);
            }
            me->m_help_line = 0;
            printHelp(me->m_help_text + me->m_help_line);

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
                                         // destroy the state-local objects...
            return Q_HANDLED();
        }
        case DOWN_SIG: {
            if (me->m_help_line + HELP_DY < me->m_help_len) {
                ++me->m_help_line;
                printHelp(me->m_help_text + me->m_help_line);
            }
            return Q_HANDLED();
        }
        case UP_SIG: {
            if (me->m_help_line > 0) {
                --me->m_help_line;
                printHelp(me->m_help_text + me->m_help_line);
            }
            return Q_HANDLED();
        }
        case HELP_SIG: {
            return Q_TRAN(me->m_history);        // go back to the last screen
        }
    }
    return Q_SUPER(&UI_top::handler);
}
