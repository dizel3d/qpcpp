//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 06, 2008
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
#include "bsp.h"
#include "game.h"

// Q_DEFINE_THIS_FILE

// local objects -------------------------------------------------------------
class Missile : public QActive {                   // extend the QActive class
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;

public:
    Missile(void) : QActive((QStateHandler)&Missile::initial) {}

private:
    static QState initial  (Missile *me, QEvent const *e);
    static QState armed    (Missile *me, QEvent const *e);
    static QState flying   (Missile *me, QEvent const *e);
    static QState exploding(Missile *me, QEvent const *e);
};


static Missile l_missile;    // the sole instance of the Missile active object

// Public-scope objects ------------------------------------------------------
QActive * const AO_Missile = &l_missile;                     // opaque pointer

// HSM definition ------------------------------------------------------------
//............................................................................
QState Missile::initial(Missile *me, QEvent const *) {
    me->subscribe(TIME_TICK_SIG);

    QS_OBJ_DICTIONARY(&l_missile);     // object dictionary for Missile object

    QS_FUN_DICTIONARY(&Missile::initial);      // dictionaries for Missile HSM
    QS_FUN_DICTIONARY(&Missile::armed);
    QS_FUN_DICTIONARY(&Missile::flying);
    QS_FUN_DICTIONARY(&Missile::exploding);

    QS_SIG_DICTIONARY(MISSILE_FIRE_SIG,   &l_missile);        // local signals
    QS_SIG_DICTIONARY(HIT_WALL_SIG,       &l_missile);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG, &l_missile);

    return Q_TRAN(&Missile::armed);
}
//............................................................................
QState Missile::armed(Missile *me, QEvent const *e) {
    switch (e->sig) {
        case MISSILE_FIRE_SIG: {
                                          // initialize position from the Ship
            me->m_x = ((ObjectPosEvt const *)e)->x;
            me->m_y = ((ObjectPosEvt const *)e)->y;
            return Q_TRAN(&Missile::flying);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Missile::flying(Missile *me, QEvent const *e) {
    switch (e->sig) {
        case TIME_TICK_SIG: {
            ObjectImageEvt *oie;
            if (me->m_x + GAME_MISSILE_SPEED_X < GAME_SCREEN_WIDTH) {
                me->m_x += GAME_MISSILE_SPEED_X;
                // tell the Tunnel to draw the Missile and test for wall hits
                oie = Q_NEW(ObjectImageEvt, MISSILE_IMG_SIG);
                oie->x   = me->m_x;
                oie->y   = me->m_y;
                oie->bmp = MISSILE_BMP;
                AO_Tunnel->postFIFO(oie);
            }
            else {   // Missile outside the range, make it ready to fire again
                return Q_TRAN(&Missile::armed);
            }
            return Q_HANDLED();
        }
        case HIT_WALL_SIG: {
            return Q_TRAN(&Missile::exploding);
        }
        case DESTROYED_MINE_SIG: {
            // tell the Ship the score for destroing this Mine
            AO_Ship->postFIFO(e);

               // re-arm immediately & let the destroyed Mine do the exploding
            return Q_TRAN(&Missile::armed);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Missile::exploding(Missile *me, QEvent const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_exp_ctr = 0;
            return Q_HANDLED();
        }
        case TIME_TICK_SIG: {
            if ((me->m_x >= GAME_SPEED_X) && (me->m_exp_ctr < 15)) {
                ObjectImageEvt *oie;

                ++me->m_exp_ctr;              // advance the explosion counter
                me->m_x -= GAME_SPEED_X;     // move the explosion by one step

                   // tell the Tunnel to render the current stage of Explosion
                oie = Q_NEW(ObjectImageEvt, EXPLOSION_SIG);
                oie->x   = me->m_x + 3;                  // x-pos of explosion
                oie->y   = (int8_t)((int)me->m_y - 4);                // y-pos
                oie->bmp = EXPLOSION0_BMP + (me->m_exp_ctr >> 2);
                AO_Tunnel->postFIFO(oie);
            }
            else {             // explosion finished or moved outside the game
                return Q_TRAN(&Missile::armed);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
