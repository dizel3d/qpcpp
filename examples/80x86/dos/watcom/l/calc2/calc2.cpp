//////////////////////////////////////////////////////////////////////////////
// Product: Calculator Example with inheritance of the whole state model
// Last Updated for Version: 4.0.01
// Date of the Last Update:  Sep 23, 2008
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
#include "qp_port.h"
#include "bsp.h"                                      // board support package
#include "calc2.h"

#include <stdlib.h>

Q_DEFINE_THIS_FILE

// state variables -----------------------------------------------------------
QStateHandler Calc2::state_operand2  = (QStateHandler)&Calc2::operand2;

// Ctor definition -----------------------------------------------------------
Calc2::Calc2(void) : Calc1() {
                                         // subsitute all overridden states...
    Calc1::state_operand2 = state_operand2;
}
//............................................................................
QState Calc2::operand2(Calc2 *me, QEvent const *e) {
    switch (e->sig) {
        case PERCENT_SIG: {
            double operand2 = BSP_get_value();
            switch (me->m_operator) {
                case KEY_PLUS:
                case KEY_MINUS: {
                    operand2 = me->m_operand1 * operand2 / 100.0;
                    break;
                }
                case KEY_MULT:
                case KEY_DIVIDE: {
                    operand2 /= 100.0;
                    break;
                }
                default: {
                    Q_ERROR();
                    break;
                }
            }
            if (BSP_eval(me->m_operand1, me->m_operator, operand2)) {
                return Q_TRAN(state_result);
            }
            else {
                return Q_TRAN(state_error);
            }
        }
    }
    return Q_SUPER(&Calc1::operand2);         // let Calc1 handle other events
}
