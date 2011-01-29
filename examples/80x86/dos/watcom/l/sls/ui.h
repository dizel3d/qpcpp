//////////////////////////////////////////////////////////////////////////////
// Product: UI with State-Local Storage Example
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
#ifndef ui_h
#define ui_h

enum UserSignals {
    QUIT_SIG = Q_USER_SIG,

    // insert other published signals here ...
    MAX_PUB_SIG,                                  // the last published signal

    C_SIG,
    CE_SIG,
    DIGIT_0_SIG,
    DIGIT_1_9_SIG,
    POINT_SIG,
    NEG_SIG,
    ENTER_SIG,
    UP_SIG,
    DOWN_SIG,
    HELP_SIG,

    MAX_SIG                              // the last signal (keep always last)
};

struct KeyboardEvt : public QEvent {
    uint8_t key_code;                                       // code of the key
};

//............................................................................
class UI_top : public QActive {
public:
    QStateHandler m_history;

public:
    UI_top(void) : QActive((QStateHandler)&UI_top::initial) {
    }

public:
    static QState initial(UI_top *me, QEvent const *e); // initial pseudostate
    static QState handler(UI_top *me, QEvent const *e);
    static QState final  (UI_top *me, QEvent const *e);
};
//............................................................................
class UI_mem : public UI_top {
private:
    uint8_t m_mem[90];    // maximum size of any substate (subclass) of UI_top
};
//............................................................................
class UI_num : public UI_top {
public:
    NumEntry m_num_entry;

public:
    static QState handler(UI_num *me, QEvent const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num) < sizeof(UI_mem));

//............................................................................
class UI_num_sd : public UI_num {                        // standard deviation
public:
    double m_n;
    double m_sum;
    double m_sum_sq;

public:
    static QState handler(UI_num_sd *me, QEvent const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num_sd) < sizeof(UI_mem));

//............................................................................
class UI_num_lr : public UI_num {                         // linear regression
public:
    double m_x;
    double m_n;
    double m_xsum;
    double m_xsum_sq;
    double m_ysum;
    double m_ysum_sq;
    double m_xysum;

public:
    static QState handler(UI_num_lr *me, QEvent const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num_lr) < sizeof(UI_mem));

//............................................................................
class UI_help : public UI_top {
public:
    char const * const *m_help_text;
    uint16_t m_help_len;
    uint16_t m_help_line;

public:
    static QState handler(UI_help *me, QEvent const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_help) < sizeof(UI_mem));

//----------------------------------------------------------------------------
extern QActive * const AO_UI;          // "opaque" pointer to UI Active Object

#endif
