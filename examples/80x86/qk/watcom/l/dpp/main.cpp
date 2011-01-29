//////////////////////////////////////////////////////////////////////////////
// Product: DPP example, QK version
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
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

// Local-scope objects -------------------------------------------------------
static QEvent const *l_tableQueueSto[N_PHILO];
static QEvent const *l_philoQueueSto[N_PHILO][N_PHILO];
static QSubscrList   l_subscrSto[MAX_PUB_SIG];

static union SmallEvents {
    void *min_size;
    TableEvt te;
    // other event types to go into this pool
} l_smlPoolSto[2*N_PHILO];                 // storage for the small event pool

//............................................................................
int main(int argc, char *argv[]) {

    BSP_init(argc, argv);                               // initialize the BSP

    QF::init();       // initialize the framework and the underlying RT kernel

    QS_FILTER_ON(QS_ALL_RECORDS);
    QS_FILTER_OFF(QS_QF_INT_LOCK);
    QS_FILTER_OFF(QS_QF_INT_UNLOCK);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QK_SCHEDULE);
                                                     // object dictionaries...
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_tableQueueSto);
    QS_OBJ_DICTIONARY(l_philoQueueSto[0]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[1]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[2]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[3]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[4]);

                                  // globally published signal dictionaries...
    QS_SIG_DICTIONARY(DONE_SIG,      0);
    QS_SIG_DICTIONARY(EAT_SIG,       0);
    QS_SIG_DICTIONARY(TERMINATE_SIG, 0);

    QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));     // init publish-subscribe

                                                  // initialize event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

                                                // start the active objects...
    uint8_t n;
    for (n = 0; n < N_PHILO; ++n) {
        Philo_start(n, (uint8_t)(n + 1),
                    l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]));
    }
    Table_start((uint8_t)(N_PHILO + 1),
                l_tableQueueSto, Q_DIM(l_tableQueueSto));

    QF::run();                                       // run the QF application

    return 0;
}

