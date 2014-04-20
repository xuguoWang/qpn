/*****************************************************************************
* Model: game.qm
* File:  ./ship.c
*
* This code has been generated by QM tool (see state-machine.com/qm).
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*****************************************************************************/
/*${.::ship.c} .............................................................*/
#include "qpn_port.h"
#include "bsp.h"
#include "game.h"

/* Q_DEFINE_THIS_MODULE(ship) */

#define SHIP_WIDTH  5U
#define SHIP_HEIGHT 3U

/* encapsulated delcaration of the Ship active object ----------------------*/
/*${AOs::Ship} .............................................................*/
typedef struct Ship {
/* protected: */
    QMActive super;

/* private: */
    uint8_t x;
    uint8_t y;
    uint8_t exp_ctr;
    uint16_t score;
} Ship;

/* protected: */
static QState Ship_initial(Ship * const me);
static QState Ship_active  (Ship * const me);
static QState Ship_active_i(Ship * const me);
static QMState const Ship_active_s = {
    (QMState const *)0, /* superstate (top) */
    Q_STATE_CAST(&Ship_active),
    Q_ACTION_CAST(0), /* no entry action */
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(&Ship_active_i)
};
static QState Ship_parked  (Ship * const me);
static QMState const Ship_parked_s = {
    &Ship_active_s, /* superstate */
    Q_STATE_CAST(&Ship_parked),
    Q_ACTION_CAST(0), /* no entry action */
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Ship_flying  (Ship * const me);
static QState Ship_flying_e(Ship * const me);
static QMState const Ship_flying_s = {
    &Ship_active_s, /* superstate */
    Q_STATE_CAST(&Ship_flying),
    Q_ACTION_CAST(&Ship_flying_e),
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Ship_exploding  (Ship * const me);
static QState Ship_exploding_e(Ship * const me);
static QMState const Ship_exploding_s = {
    &Ship_active_s, /* superstate */
    Q_STATE_CAST(&Ship_exploding),
    Q_ACTION_CAST(&Ship_exploding_e),
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};


/* global objects ----------------------------------------------------------*/
Ship AO_Ship;

/* Active object definition ------------------------------------------------*/
/*${AOs::Ship_ctor} ........................................................*/
void Ship_ctor(void) {
    Ship *me = &AO_Ship;
    QMActive_ctor(&me->super, Q_STATE_CAST(&Ship_initial));
    me->x = GAME_SHIP_X;
    me->y = GAME_SHIP_Y;
}
/*${AOs::Ship} .............................................................*/
/*${AOs::Ship::SM} .........................................................*/
static QState Ship_initial(Ship * const me) {
    static struct {
        QMState const *target;
        QActionHandler act[2];
    } const tatbl_ = { /* transition-action table */
        &Ship_active_s, /* target state */
        {
            Q_ACTION_CAST(&Ship_active_i), /* init.tran. */
            Q_ACTION_CAST(0) /* zero terminator */
        }
    };
    /* ${AOs::Ship::SM::initial} */
    return QM_TRAN_INIT(&tatbl_);
}
/*${AOs::Ship::SM::active} .................................................*/
/* ${AOs::Ship::SM::active::initial} */
static QState Ship_active_i(Ship * const me) {
    static QMTranActTable const tatbl_ = { /* transition-action table */
        &Ship_parked_s,
        {
            Q_ACTION_CAST(0) /* zero terminator */
        }
    };
    /* ${AOs::Ship::SM::active::initial} */
    return QM_TRAN_INIT(&tatbl_);
}
/* ${AOs::Ship::SM::active} */
static QState Ship_active(Ship * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Ship::SM::active::PLAYER_SHIP_MOVE} */
        case PLAYER_SHIP_MOVE_SIG: {
            me->x = (uint8_t)Q_PAR(me);
            me->y = (uint8_t)(Q_PAR(me) >> 8);
            status_ = QM_HANDLED();
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}
/*${AOs::Ship::SM::active::parked} .........................................*/
/* ${AOs::Ship::SM::active::parked} */
static QState Ship_parked(Ship * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Ship::SM::active::parked::TAKE_OFF} */
        case TAKE_OFF_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Ship_flying_s, /* target state */
                {
                    Q_ACTION_CAST(&Ship_flying_e), /* entry */
                    Q_ACTION_CAST(0) /* zero terminator */
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}
/*${AOs::Ship::SM::active::flying} .........................................*/
/* ${AOs::Ship::SM::active::flying} */
static QState Ship_flying_e(Ship * const me) {
    me->score = 0; /* reset the score */
    QACTIVE_POST((QActive *)&AO_Tunnel, SCORE_SIG, me->score);
    return QM_ENTRY(&Ship_flying_s);
}
/* ${AOs::Ship::SM::active::flying} */
static QState Ship_flying(Ship * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Ship::SM::active::flying::TIME_TICK} */
        case TIME_TICK_SIG: {
            /* tell the Tunnel to draw the Ship and test for hits */
            QACTIVE_POST((QActive *)&AO_Tunnel, SHIP_IMG_SIG,
                         ((QParam)SHIP_BMP << 16)
                         | (QParam)me->x
                         | ((QParam)me->y << 8));

            ++me->score;  /* increment the score for surviving another tick */

            if ((me->score % 10) == 0) { /* is the score "round"? */
                QACTIVE_POST((QActive *)&AO_Tunnel, SCORE_SIG, me->score);
            }
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Ship::SM::active::flying::PLAYER_TRIGGER} */
        case PLAYER_TRIGGER_SIG: {
            QACTIVE_POST((QActive *)&AO_Missile, MISSILE_FIRE_SIG,
                         (QParam)me->x
                         | (((QParam)(me->y - 1 + SHIP_HEIGHT) & 0xFF) << 8));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Ship::SM::active::flying::DESTROYED_MINE} */
        case DESTROYED_MINE_SIG: {
            me->score += (uint16_t)Q_PAR(me);
            /* the score will be sent to the Tunnel by the next TIME_TICK */
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Ship::SM::active::flying::HIT_WALL} */
        case HIT_WALL_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Ship_exploding_s, /* target state */
                {
                    Q_ACTION_CAST(&Ship_exploding_e), /* entry */
                    Q_ACTION_CAST(0) /* zero terminator */
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        /* ${AOs::Ship::SM::active::flying::HIT_MINE} */
        case HIT_MINE_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Ship_exploding_s, /* target state */
                {
                    Q_ACTION_CAST(&Ship_exploding_e), /* entry */
                    Q_ACTION_CAST(0) /* zero terminator */
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}
/*${AOs::Ship::SM::active::exploding} ......................................*/
/* ${AOs::Ship::SM::active::exploding} */
static QState Ship_exploding_e(Ship * const me) {
    me->exp_ctr = 0;
    return QM_ENTRY(&Ship_exploding_s);
}
/* ${AOs::Ship::SM::active::exploding} */
static QState Ship_exploding(Ship * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Ship::SM::active::exploding::TIME_TICK} */
        case TIME_TICK_SIG: {
            /* ${AOs::Ship::SM::active::exploding::TIME_TICK::[me->exp_ctr<15]} */
            if (me->exp_ctr < 15) {
                ++me->exp_ctr;

                  /* tell the Tunnel to draw the current stage of Explosion */
                QACTIVE_POST((QActive *)&AO_Tunnel, EXPLOSION_SIG,
                         ((QParam)(EXPLOSION0_BMP + (me->exp_ctr >> 2)) << 16)
                         | (QParam)me->x
                         | (((QParam)(me->y - 4 + SHIP_HEIGHT) & 0xFF) << 8));
                status_ = QM_HANDLED();
            }
            /* ${AOs::Ship::SM::active::exploding::TIME_TICK::[else]} */
            else {
                static QMTranActTable const tatbl_ = { /* transition-action table */
                    &Ship_parked_s,
                    {
                        Q_ACTION_CAST(0) /* zero terminator */
                    }
                };
                QACTIVE_POST((QActive *)&AO_Tunnel, GAME_OVER_SIG, me->score);
                status_ = QM_TRAN(&tatbl_);
            }
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}

