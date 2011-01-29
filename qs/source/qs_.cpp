//////////////////////////////////////////////////////////////////////////////
// Product: QS/C++
// Last Updated for Version: 4.0.01
// Date of the Last Update:  Jun 09, 2008
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
#include "qs_pkg.h"

/// \file
/// \ingroup qs
/// \brief QS functions for internal use inside QP components

//............................................................................
void const *QS::smObj_;                  // local state machine for QEP filter
void const *QS::aoObj_;                   // local active object for QF filter
void const *QS::mpObj_;                     //  local event pool for QF filter
void const *QS::eqObj_;                      //  local raw queue for QF filter
void const *QS::teObj_;                     //  local time event for QF filter
void const *QS::apObj_;                    //  local object Application filter

QSTimeCtr volatile QS::tickCtr_;     // tick counter for the QS_QF_TICK record

//............................................................................
void QS::u8_(uint8_t d) {
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16_(uint16_t d) {
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
void QS::u32_(uint32_t d) {
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_(char const *s) {
    while (*s != '\0') {
                                       // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)(*s));
        QS_INSERT_BYTE((uint8_t)(*s))
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_ROM_(char const Q_ROM * Q_ROM_VAR s) {
    uint8_t b;
    while ((b = (uint8_t)Q_ROM_BYTE(*s)) != (uint8_t)0) {
                                       // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + b);
        QS_INSERT_BYTE(b)
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
