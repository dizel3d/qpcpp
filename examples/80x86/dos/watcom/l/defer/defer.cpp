//////////////////////////////////////////////////////////////////////////////
// Product: Deferred Event state pattern example
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
#include "qp_port.h"
#include "bsp.h"

#include <stdio.h>              // this example uses printf() to report status

Q_DEFINE_THIS_FILE

//............................................................................
enum TServerSignals {
    NEW_REQUEST_SIG = Q_USER_SIG,                    // the new request signal
    RECEIVED_SIG,                             // the request has been received
    AUTHORIZED_SIG,                         // the request has been authorized
    TERMINATE_SIG                                 // terminate the application
};
//............................................................................
struct RequestEvt : public QEvent {
    uint8_t ref_num;                        // reference number of the request
};

class TServer : public QActive {           // Transaction Server active object
private:
    QEQueue m_requestQueue;     // native QF queue for deferred request events
    QEvent const *m_requestQSto[3];   // storage for the deferred queue buffer

    QTimeEvt m_receivedEvt;                    // private time event generator
    QTimeEvt m_authorizedEvt;                  // private time event generator

public:
    TServer()                                              // the default ctor
      : QActive((QStateHandler)&TServer::initial),
        m_receivedEvt(RECEIVED_SIG),
        m_authorizedEvt(AUTHORIZED_SIG)
    {
        m_requestQueue.init(m_requestQSto, Q_DIM(m_requestQSto));
    }

private:
                                             // hierarchical state machine ...
    static QState initial    (TServer *me, QEvent const *e);
    static QState idle       (TServer *me, QEvent const *e);
    static QState busy       (TServer *me, QEvent const *e);
    static QState receiving  (TServer *me, QEvent const *e);
    static QState authorizing(TServer *me, QEvent const *e);
    static QState final      (TServer *me, QEvent const *e);
};

// HSM definition ------------------------------------------------------------
QState TServer::initial(TServer *me, QEvent const *) {
    return Q_TRAN(&TServer::idle);
}
//............................................................................
QState TServer::final(TServer *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("final-ENTRY;\nBye!Bye!\n");
            QF::stop();                           // terminate the application
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState TServer::idle(TServer *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            RequestEvt const *rq;
            printf("idle-ENTRY;\n");

            // recall the request from the requestQueue */
            rq = (RequestEvt const *)me->recall(&me->m_requestQueue);
            if (rq != (RequestEvt *)0) {
                printf("Request #%d recalled\n", (int)rq->ref_num);
            }
            else {
                printf("No deferred requests\n");
            }
            return Q_HANDLED();
        }
        case NEW_REQUEST_SIG: {
            printf("Processing request #%d\n",
                   (int)((RequestEvt const *)e)->ref_num);
            return Q_TRAN(&TServer::receiving);
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&TServer::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState TServer::busy(TServer *me, QEvent const *e) {
    switch (e->sig) {
        case NEW_REQUEST_SIG: {
            if (me->m_requestQueue.getNFree() > 0) {             // can defer?
                                                          // defer the request
                me->defer(&me->m_requestQueue, e);
                printf("Request #%d deferred;\n",
                       (int)((RequestEvt const *)e)->ref_num);
            }
            else {
                printf("Request #%d IGNORED;\n",
                       (int)((RequestEvt const *)e)->ref_num);
                   // notify the request sender that his request was denied...
            }
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&TServer::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState TServer::receiving(TServer *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("receiving-ENTRY;\n");
                                               // one-shot timeout in 1 second
            me->m_receivedEvt.postIn(me, BSP_TICKS_PER_SEC);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_receivedEvt.disarm();
            return Q_HANDLED();
        }
        case RECEIVED_SIG: {
            return Q_TRAN(&TServer::authorizing);
        }
    }
    return Q_SUPER(&TServer::busy);
}
//............................................................................
QState TServer::authorizing(TServer *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("authorizing-ENTRY;\n");
                                              // one-shot timeout in 2 seconds
            me->m_authorizedEvt.postIn(me, 2*BSP_TICKS_PER_SEC);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_authorizedEvt.disarm();
            return Q_HANDLED();
        }
        case AUTHORIZED_SIG: {
            return Q_TRAN(&TServer::idle);
        }
    }
    return Q_SUPER(&TServer::busy);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static TServer       l_tserver;        // the Transaction Server active object
static QEvent const *l_tserverQSto[10];     // Event queue storage for TServer
static RequestEvt    l_smlPoolSto[10];                     // small event pool

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQEP version: %s\nQF  version: %s\n"
           "Press n to generate a new request\n"
           "Press ESC to quit...\n",
           QEP::getVersion(), QF::getVersion());

    BSP_init(argc, argv);                                // initialize the BSP

    QF::init();         // initialize the framework and the underlying RTOS/OS


    // publish-subscribe not used, no call to QF::psInit()

                                                  // initialize event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

                                                // start the active objects...
    l_tserver.start(1, l_tserverQSto, Q_DIM(l_tserverQSto),
                   (void *)0, 0, (QEvent *)0);

    QF::run();                                       // run the QF application

    return 0;
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 49: {                                        // 'n': new request?
            static uint8_t reqCtr = 0;                   // count the requests
            RequestEvt *e = Q_NEW(RequestEvt, NEW_REQUEST_SIG);
            e->ref_num = (++reqCtr);               // set the reference number
                                     // post directly to TServer active object
            l_tserver.postFIFO(e);   // post directly to TServer active object
            break;
        }
        case 129: {                                            // ESC pressed?
            static QEvent const terminateEvt = { TERMINATE_SIG, 0};
            l_tserver.postFIFO(&terminateEvt);
            break;
        }
    }
}
