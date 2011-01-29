//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++
// Last Updated for Version: 4.1.00
// Date of the Last Update:  Oct 09, 2009
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
#ifndef qpset_h
#define qpset_h

/// \file
/// \ingroup qf qk
/// \brief platform-independent priority sets of 8 or 64 elements.
///
/// This header file must be included in those QF ports that use the
/// cooperative multitasking QF scheduler or the QK.

                      // external declarations of QF lookup tables used inline
extern uint8_t const Q_ROM Q_ROM_VAR QF_log2Lkup[256];
extern uint8_t const Q_ROM Q_ROM_VAR QF_pwr2Lkup[65];
extern uint8_t const Q_ROM Q_ROM_VAR QF_invPwr2Lkup[65];
extern uint8_t const Q_ROM Q_ROM_VAR QF_div8Lkup[65];

//////////////////////////////////////////////////////////////////////////////
/// \brief Priority Set of up to 8 elements for building various schedulers,
/// but also useful as a general set of up to 8 elements of any kind.
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 8 priority levels.
class QPSet8 {
protected:
    //////////////////////////////////////////////////////////////////////////
    /// \brief bimask representing elements of the set
    uint8_t m_bits;

public:

    /// \brief the function evaluates to TRUE if the priority set is empty,
    /// which means that no active objects are ready to run.
    uint8_t isEmpty(void) volatile {
        return (uint8_t)(m_bits == (uint8_t)0);
    }

    /// \brief the function evaluates to TRUE if the priority set has elements,
    /// which means that some active objects are ready to run.
    uint8_t notEmpty(void) volatile {
        return (uint8_t)(m_bits != (uint8_t)0);
    }

    /// \brief the function evaluates to TRUE if the priority set has the
    /// element \a n.
    uint8_t hasElement(uint8_t n) volatile {
        return (uint8_t)((m_bits & Q_ROM_BYTE(QF_pwr2Lkup[n])) != 0);
    }

    /// \brief insert element \a n into the set, n = 1..8
    void insert(uint8_t n) volatile {
        m_bits |= Q_ROM_BYTE(QF_pwr2Lkup[n]);
    }

    /// \brief remove element \a n from the set, n = 1..8
    void remove(uint8_t n) volatile {
        m_bits &= Q_ROM_BYTE(QF_invPwr2Lkup[n]);
    }

    /// \brief find the maximum element in the set,
    /// \note returns zero if the set is empty
    uint8_t findMax(void) volatile {
        return Q_ROM_BYTE(QF_log2Lkup[m_bits]);
    }

    friend class QPSet64;
};

//////////////////////////////////////////////////////////////////////////////
/// \brief Priority Set of up to 64 elements for building various schedulers,
/// but also useful as a general set of up to 64 elements of any kind.
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 64 priority levels.
///
/// The priority set allows to build cooperative multitasking schedulers
/// to manage up to 64 tasks. It is also used in the Quantum Kernel (QK)
/// preemptive scheduler.
///
/// The inherited 8-bit set is used as the 8-elemtn set of of 8-bit subsets
/// Each bit in the super.bits set represents a subset (8-elements)
/// as follows: \n
/// bit 0 in this->m_bits is 1 when subset[0] is not empty \n
/// bit 1 in this->m_bits is 1 when subset[1] is not empty \n
/// bit 2 in this->m_bits is 1 when subset[2] is not empty \n
/// bit 3 in this->m_bits is 1 when subset[3] is not empty \n
/// bit 4 in this->m_bits is 1 when subset[4] is not empty \n
/// bit 5 in this->m_bits is 1 when subset[5] is not empty \n
/// bit 6 in this->m_bits is 1 when subset[6] is not empty \n
/// bit 7 in this->m_bits is 1 when subset[7] is not empty \n
class QPSet64 : public QPSet8 {

    /// \brief subsets representing elements in the set as follows: \n
    /// m_subset[0] represent elements  1..8  \n
    /// m_subset[1] represent elements  9..16 \n
    /// m_subset[2] represent elements 17..24 \n
    /// m_subset[3] represent elements 25..32 \n
    /// m_subset[4] represent elements 33..40 \n
    /// m_subset[5] represent elements 41..48 \n
    /// m_subset[6] represent elements 49..56 \n
    /// m_subset[7] represent elements 57..64 \n
    QPSet8 m_subset[8];

public:

    /// \brief the function evaluates to TRUE if the priority set has the
    /// element \a n.
    uint8_t hasElement(uint8_t n) volatile {
         return m_subset[Q_ROM_BYTE(QF_div8Lkup[n])].QPSet8::hasElement(n);
    }

    /// \brief insert element \a n into the set, n = 1..64
    void insert(uint8_t n) volatile {
        QPSet8::insert(Q_ROM_BYTE(QF_div8Lkup[n]) + 1);
        m_subset[Q_ROM_BYTE(QF_div8Lkup[n])].QPSet8::insert(n);
    }

    /// \brief remove element \a n from the set, n = 1..64
    void remove(uint8_t n) volatile {
        if ((m_subset[Q_ROM_BYTE(QF_div8Lkup[n])].m_bits
                 &= Q_ROM_BYTE(QF_invPwr2Lkup[n])) == (uint8_t)0)
        {
            QPSet8::remove(Q_ROM_BYTE(QF_div8Lkup[n]) + 1);
        }
    }

    /// \brief find the maximum element in the set,
    /// \note returns zero if the set is empty
    uint8_t findMax(void) volatile {
        if (m_bits != (uint8_t)0) {
            uint8_t n = (uint8_t)(Q_ROM_BYTE(QF_log2Lkup[m_bits]) - 1);
            return (uint8_t)(Q_ROM_BYTE(QF_log2Lkup[m_subset[n].m_bits])
                             + (n << 3));
        }
        else {
            return (uint8_t)0;
        }
    }
};

#endif                                                              // qpset_h

