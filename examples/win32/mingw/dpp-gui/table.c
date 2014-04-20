/*****************************************************************************
* Model: dpp.qm
* File:  ./table.c
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
/*${.::table.c} ............................................................*/
#include "qpn_port.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_MODULE("table")

/* Active object class -----------------------------------------------------*/
/*${AOs::Table} ............................................................*/
typedef struct Table {
/* protected: */
    QMActive super;

/* private: */
    uint8_t fork[N_PHILO];
    uint8_t isHungry[N_PHILO];
} Table;

/* protected: */
static QState Table_initial(Table * const me);
static QState Table_active  (Table * const me);
static QMState const Table_active_s = {
    (QMState const *)0, /* superstate (top) */
    Q_STATE_CAST(&Table_active),
    Q_ACTION_CAST(0), /* no entry action */
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Table_serving  (Table * const me);
static QState Table_serving_e(Table * const me);
static QMState const Table_serving_s = {
    &Table_active_s, /* superstate */
    Q_STATE_CAST(&Table_serving),
    Q_ACTION_CAST(&Table_serving_e),
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Table_paused  (Table * const me);
static QState Table_paused_e(Table * const me);
static QState Table_paused_x(Table * const me);
static QMState const Table_paused_s = {
    &Table_active_s, /* superstate */
    Q_STATE_CAST(&Table_paused),
    Q_ACTION_CAST(&Table_paused_e),
    Q_ACTION_CAST(&Table_paused_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};


/* Global objects ----------------------------------------------------------*/
/*${AOs::AO_Table} .........................................................*/
struct Table AO_Table;


#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1U)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1U) % N_PHILO))
#define FREE      ((uint8_t)0)
#define USED      ((uint8_t)1)

/*..........................................................................*/
/*${AOs::Table_ctor} .......................................................*/
void Table_ctor(void) {
    uint8_t n;
    Table *me = &AO_Table;

    QMActive_ctor(&me->super, Q_STATE_CAST(&Table_initial));
    for (n = 0U; n < N_PHILO; ++n) {
        me->fork[n] = FREE;
        me->isHungry[n] = 0U;
    }
}
/*${AOs::Table} ............................................................*/
/*${AOs::Table::SM} ........................................................*/
static QState Table_initial(Table * const me) {
    static struct {
        QMState const *target;
        QActionHandler act[2];
    } const tatbl_ = { /* transition-action table */
        &Table_serving_s, /* target state */
        {
            Q_ACTION_CAST(&Table_serving_e), /* entry */
            Q_ACTION_CAST(0) /* zero terminator */
        }
    };
    /* ${AOs::Table::SM::initial} */
    uint8_t n;
    for (n = 0U; n < N_PHILO; ++n) {
        me->fork[n] = FREE;
        me->isHungry[n] = 0U;
        BSP_displayPhilStat(n, "thinking");
    }
    return QM_TRAN_INIT(&tatbl_);
}
/*${AOs::Table::SM::active} ................................................*/
/* ${AOs::Table::SM::active} */
static QState Table_active(Table * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Table::SM::active::TERMINATE} */
        case TERMINATE_SIG: {
            BSP_terminate(0);
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Table::SM::active::EAT} */
        case EAT_SIG: {
            Q_ERROR();
            status_ = QM_HANDLED();
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    (void)me; /* avoid compiler warning in case 'me' is not used */
    return status_;
}
/*${AOs::Table::SM::active::serving} .......................................*/
/* ${AOs::Table::SM::active::serving} */
static QState Table_serving_e(Table * const me) {
    uint8_t n;
    for (n = 0U; n < N_PHILO; ++n) { /* give permissions to eat... */
        if ((me->isHungry[n] != 0U)
            && (me->fork[LEFT(n)] == FREE)
            && (me->fork[n] == FREE))
        {
            QActive *philo;

            me->fork[LEFT(n)] = USED;
            me->fork[n] = USED;
            philo =
               (QActive *)Q_ROM_PTR(QF_active[PHILO_0_PRIO + n].act);
            QACTIVE_POST(philo, EAT_SIG, n);
            me->isHungry[n] = 0U;
            BSP_displayPhilStat(n, "eating  ");
        }
    }
    return QM_ENTRY(&Table_serving_s);
}
/* ${AOs::Table::SM::active::serving} */
static QState Table_serving(Table * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Table::SM::active::serving::HUNGRY} */
        case HUNGRY_SIG: {
            uint8_t n, m;
            QActive *philo;

            n = (uint8_t)(Q_PAR(me) - PHILO_0_PRIO);
            /* phil ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (me->isHungry[n] == 0U));

            BSP_displayPhilStat(n, "hungry  ");
            m = LEFT(n);
            /* ${AOs::Table::SM::active::serving::HUNGRY::[bothfree]} */
            if ((me->fork[m] == FREE) && (me->fork[n] == FREE)) {
                me->fork[m] = USED;
                me->fork[n] = USED;
                philo = (QActive *)Q_ROM_PTR(QF_active[PHILO_0_PRIO + n].act);
                QACTIVE_POST(philo, EAT_SIG, n);
                BSP_displayPhilStat(n, "eating  ");
                status_ = QM_HANDLED();
            }
            /* ${AOs::Table::SM::active::serving::HUNGRY::[else]} */
            else {
                me->isHungry[n] = 1U;
                status_ = QM_HANDLED();
            }
            break;
        }
        /* ${AOs::Table::SM::active::serving::DONE} */
        case DONE_SIG: {
            uint8_t n, m;
            QActive *philo;

            n = (uint8_t)(Q_PAR(me) - PHILO_0_PRIO);
            /* phil ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (me->isHungry[n] == 0U));

            BSP_displayPhilStat(n, "thinking");
            m = LEFT(n);
            /* both forks of Phil[n] must be used */
            Q_ASSERT((me->fork[n] == USED) && (me->fork[m] == USED));

            me->fork[m] = FREE;
            me->fork[n] = FREE;
            m = RIGHT(n); /* check the right neighbor */

            if ((me->isHungry[m] != 0U) && (me->fork[m] == FREE)) {
                me->fork[n] = USED;
                me->fork[m] = USED;
                me->isHungry[m] = 0U;
                philo = (QActive *)Q_ROM_PTR(QF_active[PHILO_0_PRIO + m].act);
                QACTIVE_POST(philo, EAT_SIG, m);
                BSP_displayPhilStat(m, "eating  ");
            }
            m = LEFT(n); /* check the left neighbor */
            n = LEFT(m); /* left fork of the left neighbor */
            if ((me->isHungry[m] != 0U) && (me->fork[n] == FREE)) {
                me->fork[m] = USED;
                me->fork[n] = USED;
                me->isHungry[m] = 0U;
                philo = (QActive *)Q_ROM_PTR(QF_active[PHILO_0_PRIO + m].act);
                QACTIVE_POST(philo, EAT_SIG, m);
                BSP_displayPhilStat(m, "eating  ");
            }
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Table::SM::active::serving::EAT} */
        case EAT_SIG: {
            Q_ERROR();
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Table::SM::active::serving::PAUSE} */
        case PAUSE_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Table_paused_s, /* target state */
                {
                    Q_ACTION_CAST(&Table_paused_e), /* entry */
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
/*${AOs::Table::SM::active::paused} ........................................*/
/* ${AOs::Table::SM::active::paused} */
static QState Table_paused_e(Table * const me) {
    BSP_displayPaused(1U);
    (void)me; /* avoid compiler warning in case 'me' is not used */
    return QM_ENTRY(&Table_paused_s);
}
/* ${AOs::Table::SM::active::paused} */
static QState Table_paused_x(Table * const me) {
    BSP_displayPaused(0U);
    (void)me; /* avoid compiler warning in case 'me' is not used */
    return QM_EXIT(&Table_paused_s);
}
/* ${AOs::Table::SM::active::paused} */
static QState Table_paused(Table * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Table::SM::active::paused::PAUSE} */
        case PAUSE_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[3];
            } const tatbl_ = { /* transition-action table */
                &Table_serving_s, /* target state */
                {
                    Q_ACTION_CAST(&Table_paused_x), /* exit */
                    Q_ACTION_CAST(&Table_serving_e), /* entry */
                    Q_ACTION_CAST(0) /* zero terminator */
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        /* ${AOs::Table::SM::active::paused::HUNGRY} */
        case HUNGRY_SIG: {
            uint8_t n = (uint8_t)(Q_PAR(me) - PHILO_0_PRIO);
            /* philo ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (me->isHungry[n] == 0U));
            me->isHungry[n] = 1U;
            BSP_displayPhilStat(n, "hungry  ");
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Table::SM::active::paused::DONE} */
        case DONE_SIG: {
            uint8_t n, m;

            n = (uint8_t)(Q_PAR(me) - PHILO_0_PRIO);
            /* phil ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (me->isHungry[n] == 0U));

            BSP_displayPhilStat(n, "thinking");
            m = LEFT(n);
            /* both forks of Phil[n] must be used */
            Q_ASSERT((me->fork[n] == USED) && (me->fork[m] == USED));

            me->fork[m] = FREE;
            me->fork[n] = FREE;
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

