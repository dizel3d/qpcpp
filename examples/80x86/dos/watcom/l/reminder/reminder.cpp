//////////////////////////////////////////////////////////////////////////////
// Product: Reminder state pattern example
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
#include "qf_port.h"
#include "bsp.h"
#include "qassert.h"

#include <stdio.h>

Q_DEFINE_THIS_FILE

//............................................................................
enum SensorSignals {
    TERMINATE_SIG = Q_USER_SIG,                   // terminate the application
    TIMEOUT_SIG,                                // the periodic timeout signal
    DATA_READY_SIG,                            // the invented reminder signal

    MAX_SIG                                                // keep always last
};
//............................................................................
class Sensor : public QActive {                    // the Sensor active object
private:
    QTimeEvt m_timeEvt;                        // private time event generator
    uint16_t m_pollCtr;
    uint16_t m_procCtr;

public:
    Sensor()
        : QActive((QStateHandler)&Sensor::initial),
          m_timeEvt(TIMEOUT_SIG)
    {}

private:
                                             // hierarchical state machine ...
    static QState initial   (Sensor *me, QEvent const *e);
    static QState polling   (Sensor *me, QEvent const *e);
    static QState processing(Sensor *me, QEvent const *e);
    static QState idle      (Sensor *me, QEvent const *e);
    static QState busy      (Sensor *me, QEvent const *e);
    static QState final     (Sensor *me, QEvent const *e);
};

// HSM definition ------------------------------------------------------------
QState Sensor::initial(Sensor *me, QEvent const *) {
    me->m_pollCtr = 0;
    me->m_procCtr = 0;

    // NOTE: don't forget to subscribe to any signals of interest,
    // if you're using publish-subscribe...
    //
    // subscribe(...);

    return Q_TRAN(&Sensor::polling);
}
//............................................................................
QState Sensor::final(Sensor *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> final\n");
            printf("\nBye!Bye!\n");
            QF::stop();                                 // stop QF and cleanup
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Sensor::polling(Sensor *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                                          // periodic timeout every 1/2 second
            me->m_timeEvt.fireEvery(me, BSP_TICKS_PER_SEC/2);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_timeEvt.disarm();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Sensor::processing);
        }
        case TIMEOUT_SIG: {
            // NOTE: this constant event is statically pre-allocated.
            // It can be posted/published as any other event.
            static const QEvent dataReadyEvt = { DATA_READY_SIG, 0 };

            ++me->m_pollCtr;
            printf("poll %3d\n", me->m_pollCtr);
            if ((me->m_pollCtr & 0x3) == 0) {                      // modulo 4
                me->postFIFO(&dataReadyEvt);
            }
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&Sensor::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Sensor::processing(Sensor *me, QEvent const *e) {
    switch (e->sig) {
        case Q_INIT_SIG: {
            return Q_TRAN(&Sensor::idle);
        }
    }
    return Q_SUPER(&Sensor::polling);
}
//............................................................................
QState Sensor::idle(Sensor *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> idle\n");
            return Q_HANDLED();
        }
        case DATA_READY_SIG: {
            return Q_TRAN(&Sensor::busy);
        }
    }
    return Q_SUPER(&Sensor::processing);
}
//............................................................................
QState Sensor::busy(Sensor *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> busy\n");
            return Q_HANDLED();
        }
        case TIMEOUT_SIG: {
            ++me->m_procCtr;
            printf("process %3d\n", me->m_procCtr);
            if ((me->m_procCtr & 0x1) == 0) {                      // modulo 2
                return Q_TRAN(&Sensor::idle);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Sensor::processing);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static Sensor l_sensor;                            // the Sensor active object
static QEvent const *l_sensorQSto[10];       // Event queue storage for Sensor
static void *l_regPoolSto[100/sizeof(void *)]; // 100 bytes for the event pool

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQEP version: %s\nQF  version: %s\n"
           "Press ESC to quit...\n",
           QEP::getVersion(), QF::getVersion());

    BSP_init(argc, argv);                                // initialize the BSP

    QF::init();       // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QF::psInit()

                                                  // initialize event pools...
    QF::poolInit(l_regPoolSto, sizeof(l_regPoolSto), sizeof(QEvent));

                                                // start the active objects...
    l_sensor.start(1, l_sensorQSto, Q_DIM(l_sensorQSto),
                   (void *)0, 0, (QEvent *)0);

    QF::run();                                       // run the QF application

    return 0;
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 129: {                                            // ESC pressed?
            l_sensor.postFIFO(Q_NEW(QEvent, TERMINATE_SIG));
            break;
        }
    }
}
