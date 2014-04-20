/*****************************************************************************
* Model: game.qm
* File:  ./tunnel.c
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
/*${.::tunnel.c} ...........................................................*/
#include "qpn_port.h"
#include "game.h"
#include "bsp.h"

#include <string.h>                           /* for memmove() and memcpy() */

Q_DEFINE_THIS_MODULE("tunnel")

/* local objects -----------------------------------------------------------*/
/*${AOs::Tunnel} ...........................................................*/
typedef struct Tunnel {
/* protected: */
    QMActive super;

/* private: */
    QMsm * mines[GAME_MINES_MAX];
    QMsm * mine1_pool[GAME_MINES_MAX];
    QMsm * mine2_pool[GAME_MINES_MAX];
    uint8_t blink_ctr;
    uint8_t last_mine_x;
    uint8_t last_mine_y;
    uint8_t wall_thickness_top;
    uint8_t wall_thickness_bottom;
    uint8_t minimal_gap;
} Tunnel;

/* private: */
static void Tunnel_advance(Tunnel * const me);
static void Tunnel_plantMine(Tunnel * const me);
static void Tunnel_addImageAt(
    Tunnel * const me,
    uint8_t bmp,
    uint8_t x_pos,
    int8_t y_pos);
static void Tunnel_dispatchToAllMines(Tunnel * const me, QSignal sig, QParam par);
static uint8_t Tunnel_isWallHit(
    Tunnel * const me,
    uint8_t bmp,
    uint8_t x_pos,
    uint8_t y_pos);

/* protected: */
static QState Tunnel_initial(Tunnel * const me);
static QState Tunnel_active  (Tunnel * const me);
static QMState const Tunnel_active_s = {
    (QMState const *)0, /* superstate (top) */
    Q_STATE_CAST(&Tunnel_active),
    Q_ACTION_CAST(0), /* no entry action */
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_demo  (Tunnel * const me);
static QState Tunnel_demo_e(Tunnel * const me);
static QState Tunnel_demo_x(Tunnel * const me);
static QMState const Tunnel_demo_s = {
    &Tunnel_active_s, /* superstate */
    Q_STATE_CAST(&Tunnel_demo),
    Q_ACTION_CAST(&Tunnel_demo_e),
    Q_ACTION_CAST(&Tunnel_demo_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_playing  (Tunnel * const me);
static QState Tunnel_playing_e(Tunnel * const me);
static QState Tunnel_playing_x(Tunnel * const me);
static QMState const Tunnel_playing_s = {
    &Tunnel_active_s, /* superstate */
    Q_STATE_CAST(&Tunnel_playing),
    Q_ACTION_CAST(&Tunnel_playing_e),
    Q_ACTION_CAST(&Tunnel_playing_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_screen_saver  (Tunnel * const me);
static QState Tunnel_screen_saver_i(Tunnel * const me);
static QMState const Tunnel_screen_saver_s = {
    &Tunnel_active_s, /* superstate */
    Q_STATE_CAST(&Tunnel_screen_saver),
    Q_ACTION_CAST(0), /* no entry action */
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(&Tunnel_screen_saver_i)
};
static QState Tunnel_screen_saver_hide  (Tunnel * const me);
static QState Tunnel_screen_saver_hide_e(Tunnel * const me);
static QState Tunnel_screen_saver_hide_x(Tunnel * const me);
static QMState const Tunnel_screen_saver_hide_s = {
    &Tunnel_screen_saver_s, /* superstate */
    Q_STATE_CAST(&Tunnel_screen_saver_hide),
    Q_ACTION_CAST(&Tunnel_screen_saver_hide_e),
    Q_ACTION_CAST(&Tunnel_screen_saver_hide_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_screen_saver_show  (Tunnel * const me);
static QState Tunnel_screen_saver_show_e(Tunnel * const me);
static QState Tunnel_screen_saver_show_x(Tunnel * const me);
static QMState const Tunnel_screen_saver_show_s = {
    &Tunnel_screen_saver_s, /* superstate */
    Q_STATE_CAST(&Tunnel_screen_saver_show),
    Q_ACTION_CAST(&Tunnel_screen_saver_show_e),
    Q_ACTION_CAST(&Tunnel_screen_saver_show_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_game_over  (Tunnel * const me);
static QState Tunnel_game_over_e(Tunnel * const me);
static QState Tunnel_game_over_x(Tunnel * const me);
static QMState const Tunnel_game_over_s = {
    &Tunnel_active_s, /* superstate */
    Q_STATE_CAST(&Tunnel_game_over),
    Q_ACTION_CAST(&Tunnel_game_over_e),
    Q_ACTION_CAST(&Tunnel_game_over_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_show_logo  (Tunnel * const me);
static QState Tunnel_show_logo_e(Tunnel * const me);
static QState Tunnel_show_logo_x(Tunnel * const me);
static QMState const Tunnel_show_logo_s = {
    &Tunnel_active_s, /* superstate */
    Q_STATE_CAST(&Tunnel_show_logo),
    Q_ACTION_CAST(&Tunnel_show_logo_e),
    Q_ACTION_CAST(&Tunnel_show_logo_x),
    Q_ACTION_CAST(0)  /* no intitial tran. */
};
static QState Tunnel_final  (Tunnel * const me);
static QState Tunnel_final_e(Tunnel * const me);
static QMState const Tunnel_final_s = {
    (QMState const *)0, /* superstate (top) */
    Q_STATE_CAST(&Tunnel_final),
    Q_ACTION_CAST(&Tunnel_final_e),
    Q_ACTION_CAST(0), /* no exit action */
    Q_ACTION_CAST(0)  /* no intitial tran. */
};


static uint32_t l_rnd;     /* random seed */
static uint8_t l_walls[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8];
static uint8_t l_frame[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8];

/* local helper functions --------------------------------------------------*/
static void randomSeed(uint32_t seed);
static uint32_t random(void);

/* global objects ----------------------------------------------------------*/
Tunnel AO_Tunnel;

/* helper functions --------------------------------------------------------*/
/*
* The bitmap for the "Press Button" text:
*
*     xxx.........................xxx........x...x...........
*     x..x........................x..x.......x...x...........
*     x..x.x.xx..xx...xxx..xxx....x..x.x..x.xxx.xxx..xx..xxx.
*     xxx..xx...x..x.x....x.......xxx..x..x..x...x..x..x.x..x
*     x....x....xxxx..xx...xx.....x..x.x..x..x...x..x..x.x..x
*     x....x....x.......x....x....x..x.x..x..x...x..x..x.x..x
*     x....x.....xxx.xxx..xxx.....xxx...xxx...x...x..xx..x..x
*     .......................................................
*/
static uint8_t const press_button_bits[] = {
    0x7F, 0x09, 0x09, 0x06, 0x00, 0x7C, 0x08, 0x04, 0x04, 0x00,
    0x38, 0x54, 0x54, 0x58, 0x00, 0x48, 0x54, 0x54, 0x24, 0x00,
    0x48, 0x54, 0x54, 0x24, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x49,
    0x49, 0x36, 0x00, 0x3C, 0x40, 0x40, 0x7C, 0x00, 0x04, 0x3F,
    0x44, 0x00, 0x04, 0x3F, 0x44, 0x00, 0x38, 0x44, 0x44, 0x38,
    0x00, 0x7C, 0x04, 0x04, 0x78
};

/* bitmap of the Ship:
*
*     x....
*     xxx..
*     xxxxx
*/
static uint8_t const ship_bits[] = {
    0x07, 0x06, 0x06, 0x04, 0x04
};

/* bitmap of the Missile:
*
*     xxx
*/
static uint8_t const missile_bits[] = {
    0x01, 0x01, 0x01
};

/* bitmap of the Mine type-1:
*
*     .x.
*     xxx
*     .x.
*/
static uint8_t const mine1_bits[] = {
    0x02, 0x07, 0x02
};

/* bitmap of the Mine type-2:
*
*     x..x
*     .xx.
*     .xx.
*     x..x
*/
static uint8_t const mine2_bits[] = {
    0x09, 0x06, 0x06, 0x09
};

/* Mine type-2 is nastier than Mine type-1. The type-2 mine can
* hit the Ship with any of its "tentacles". However, it can be
* destroyed by the Missile only by hitting its center, defined as
* the following bitmap:
*
*     ....
*     .xx.
*     .xx.
*     ....
*/
static uint8_t const mine2_missile_bits[] = {
    0x00, 0x06, 0x06, 0x00
};

/*
* The bitmap of the explosion stage 0:
*
*     .......
*     .......
*     ...x...
*     ..x.x..
*     ...x...
*     .......
*     .......
*/
static uint8_t const explosion0_bits[] = {
    0x00, 0x00, 0x08, 0x14, 0x08, 0x00, 0x00
};

/*
* The bitmap of the explosion stage 1:
*
*     .......
*     .......
*     ..x.x..
*     ...x...
*     ..x.x..
*     .......
*     .......
*/
static uint8_t const explosion1_bits[] = {
    0x00, 0x00, 0x14, 0x08, 0x14, 0x00, 0x00
};

/*
* The bitmap of the explosion stage 2:
*
*     .......
*     .x...x.
*     ..x.x..
*     ...x...
*     ..x.x..
*     .x...x.
*     .......
*/
static uint8_t const explosion2_bits[] = {
    0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00
};

/*
* The bitmap of the explosion stage 3:
*
*     x..x..x
*     .x.x.x.
*     ..x.x..
*     xx.x.xx
*     ..x.x..
*     .x.x.x.
*     x..x..x
*/
static uint8_t const explosion3_bits[] = {
    0x49, 0x2A, 0x14, 0x6B, 0x14, 0x2A, 0x49
};

typedef struct BitmapTag { /* the auxiliary structure to hold const bitmaps */
    uint8_t const *bits;                          /* the bits in the bitmap */
    uint8_t width;                               /* the width of the bitmap */
} Bitmap;

static Bitmap const l_bitmap[MAX_BMP] = {
    { press_button_bits,  Q_DIM(press_button_bits)  },
    { ship_bits,          Q_DIM(ship_bits)          },
    { missile_bits,       Q_DIM(missile_bits)       },
    { mine1_bits,         Q_DIM(mine1_bits)         },
    { mine2_bits,         Q_DIM(mine2_bits)         },
    { mine2_missile_bits, Q_DIM(mine2_missile_bits) },
    { explosion0_bits,    Q_DIM(explosion0_bits)    },
    { explosion1_bits,    Q_DIM(explosion1_bits)    },
    { explosion2_bits,    Q_DIM(explosion2_bits)    },
    { explosion3_bits,    Q_DIM(explosion3_bits)    }
};

/* Active object definition ================================================*/
/*${AOs::Tunnel_ctor} ......................................................*/
void Tunnel_ctor(void) {
    uint8_t n;
    Tunnel *me = &AO_Tunnel;
    QMActive_ctor(&me->super, Q_STATE_CAST(&Tunnel_initial));
    for (n = 0; n < GAME_MINES_MAX; ++n) {
        me->mine1_pool[n] = Mine1_ctor(n); /* instantiate Mine1 in the pool */
        me->mine2_pool[n] = Mine2_ctor(n); /* instantiate Mine2 in the pool */
        me->mines[n] = (QMsm *)0;                     /* mine 'n' is unused */
    }
    me->last_mine_x = 0;   /* the last mine at the right edge of the tunnel */
    me->last_mine_y = 0;
}
/*${AOs::Tunnel} ...........................................................*/
/*${AOs::Tunnel::advance} ..................................................*/
static void Tunnel_advance(Tunnel * const me) {
    uint32_t rnd;
    uint32_t bmp1; /* bimap representing 1 column of the image */

    rnd = (random() & 0xFFU);

    /* reduce the top wall thickness 18.75% of the time */
    if ((rnd < 48U) && (me->wall_thickness_top > 0U)) {
        --me->wall_thickness_top;
    }

    /* reduce the bottom wall thickness 18.75% of the time */
    if ((rnd > 208U) && (me->wall_thickness_bottom > 0U)) {
        --me->wall_thickness_bottom;
    }

    rnd = (random() & 0xFFU);

    /* grow the top wall thickness 18.75% of the time */
    if ((rnd < 48U)
        && ((GAME_SCREEN_HEIGHT
             - me->wall_thickness_top
             - me->wall_thickness_bottom) > me->minimal_gap)
        && ((me->last_mine_x < (GAME_SCREEN_WIDTH - 5U))
            || (me->last_mine_y > (me->wall_thickness_top + 1U))))
    {
        ++me->wall_thickness_top;
    }

    /* grow the bottom wall thickness 18.75% of the time */
    if ((rnd > 208U)
        && ((GAME_SCREEN_HEIGHT
             - me->wall_thickness_top
             - me->wall_thickness_bottom) > me->minimal_gap)
        && ((me->last_mine_x < (GAME_SCREEN_WIDTH - 5U))
             || (me->last_mine_y + 1U
                < (GAME_SCREEN_HEIGHT - me->wall_thickness_bottom))))
    {
        ++me->wall_thickness_bottom;
    }

    /* advance the Tunnel by 1 game step to the left */
    memmove(l_walls, l_walls + GAME_SPEED_X,
            (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8) - GAME_SPEED_X);

    bmp1 = (~(~0 << me->wall_thickness_top))
            | (~0 << (GAME_SCREEN_HEIGHT
                        - me->wall_thickness_bottom));

    l_walls[GAME_SCREEN_WIDTH - 1] = (uint8_t)bmp1;
    l_walls[GAME_SCREEN_WIDTH + GAME_SCREEN_WIDTH - 1]
          = (uint8_t)(bmp1 >> 8);

    /* copy the Tunnel layer to the main frame buffer */
    memcpy(l_frame, l_walls, (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
}
/*${AOs::Tunnel::plantMine} ................................................*/
static void Tunnel_plantMine(Tunnel * const me) {
    uint32_t rnd = (random() & 0xFFU);

    if (me->last_mine_x > 0U) {
        --me->last_mine_x;    /* shift the last Mine 1 position to the left */
    }
                                                   /* last mine far enough? */
    if ((me->last_mine_x + GAME_MINES_DIST_MIN < GAME_SCREEN_WIDTH)
        && (rnd < 8))                /* place the mines only 5% of the time */
    {
        uint8_t n;
        for (n = 0; n < Q_DIM(me->mines); ++n) { /* look for disabled mines */
            if (me->mines[n] == (QMsm *)0) {
                break;
            }
        }
        if (n < Q_DIM(me->mines)) {               /* a disabled Mine found? */
            rnd = (random() & 0xFFFFU);

            if ((rnd & 1U) == 0U) {          /* choose the type of the mine */
                me->mines[n] = me->mine1_pool[n];
            }
            else {
                me->mines[n] = me->mine2_pool[n];
            }

            /* new Mine is planted in the last column of the tunnel */
            me->last_mine_x = GAME_SCREEN_WIDTH;

            /* choose a random y-position for the Mine in the Tunnel */
            rnd %= (GAME_SCREEN_HEIGHT
                    - me->wall_thickness_top
                    - me->wall_thickness_bottom - 4U);
            me->last_mine_y = (uint8_t)(me->wall_thickness_top + 2U + rnd);

            Q_SIG(me->mines[n]) = MINE_PLANT_SIG;
            Q_PAR(me->mines[n]) = (me->last_mine_x | (me->last_mine_y << 8));
            QMSM_DISPATCH(me->mines[n]); /* direct dispatch */
        }
    }
}
/*${AOs::Tunnel::addImageAt} ...............................................*/
static void Tunnel_addImageAt(
    Tunnel * const me,
    uint8_t bmp,
    uint8_t x_pos,
    int8_t y_pos)
{
    uint8_t x; /* the x-index of the image */
    uint8_t w; /* the width of the image */

    Q_REQUIRE(bmp < Q_DIM(l_bitmap));

    w = l_bitmap[bmp].width;
    if (w > GAME_SCREEN_WIDTH - x_pos) {
        w = GAME_SCREEN_WIDTH - x_pos;
    }
    for (x = 0; x < w; ++x) {
        uint32_t bmp1;
        if (y_pos >= 0) {
            bmp1 = (l_bitmap[bmp].bits[x] << (uint8_t)y_pos);
        }
        else {
            bmp1 = (l_bitmap[bmp].bits[x] >> (uint8_t)(-y_pos));
        }
        l_frame[x_pos + x] |= (uint8_t)bmp1;
        l_frame[x_pos + x + GAME_SCREEN_WIDTH] |= (uint8_t)(bmp1 >> 8);
    }
    (void)me; /* avoid the compiler warning */
}
/*${AOs::Tunnel::dispatchToAllMines} .......................................*/
static void Tunnel_dispatchToAllMines(Tunnel * const me, QSignal sig, QParam par) {
    uint8_t n;

    for (n = 0; n < GAME_MINES_MAX; ++n) {
        if (me->mines[n] != (QMsm *)0) { /* is the mine used? */
            Q_SIG(me->mines[n]) = sig;
            Q_PAR(me->mines[n]) = par;
            QMSM_DISPATCH(me->mines[n]);
        }
    }
}
/*${AOs::Tunnel::isWallHit} ................................................*/
static uint8_t Tunnel_isWallHit(
    Tunnel * const me,
    uint8_t bmp,
    uint8_t x_pos,
    uint8_t y_pos)
{
    uint8_t x;
    uint8_t w; /* the width of the image */

    Q_REQUIRE(bmp < Q_DIM(l_bitmap));

    w = l_bitmap[bmp].width;
    if (w > GAME_SCREEN_WIDTH - x_pos) {
        w = GAME_SCREEN_WIDTH - x_pos;
    }
    for (x = 0; x < w; ++x) {
        uint32_t bmp1 = ((uint32_t)l_bitmap[bmp].bits[x] << y_pos);
        if (((l_walls[x_pos + x] & (uint8_t)bmp1) != 0)
            || ((l_walls[x_pos + x + GAME_SCREEN_WIDTH]
                 & (uint8_t)(bmp1 >> 8)) != 0))
        {
            return (uint8_t)1;
        }
    }
    (void)me;  /* avoid the compiler warning */
    return (uint8_t)0;
}
/*${AOs::Tunnel::SM} .......................................................*/
static QState Tunnel_initial(Tunnel * const me) {
    static struct {
        QMState const *target;
        QActionHandler act[2];
    } const tatbl_ = { /* transition-action table */
        &Tunnel_show_logo_s, /* target state */
        {
            Q_ACTION_CAST(&Tunnel_show_logo_e), /* entry */
            Q_ACTION_CAST(0) /* zero terminator */
        }
    };
    /* ${AOs::Tunnel::SM::initial} */
    uint8_t n;
    for (n = 0; n < GAME_MINES_MAX; ++n) {
        QMSM_INIT(me->mine1_pool[n]); /* initial tran. for Mine1 */
        QMSM_INIT(me->mine2_pool[n]); /* initial tran. for Mine2 */
    }

    randomSeed(1234); /* seed the pseudo-random generator */
    return QM_TRAN_INIT(&tatbl_);
}
/*${AOs::Tunnel::SM::active} ...............................................*/
/* ${AOs::Tunnel::SM::active} */
static QState Tunnel_active(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::MINE_DISABLED} */
        case MINE_DISABLED_SIG: {
            Q_ASSERT((Q_PAR(me) < GAME_MINES_MAX)
                     && (me->mines[Q_PAR(me)] != (QMsm *)0));
            me->mines[Q_PAR(me)] = (QMsm *)0;
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::PLAYER_QUIT} */
        case PLAYER_QUIT_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_final_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_final_e), /* entry */
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
/*${AOs::Tunnel::SM::active::demo} .........................................*/
/* ${AOs::Tunnel::SM::active::demo} */
static QState Tunnel_demo_e(Tunnel * const me) {
    me->last_mine_x = 0; /* last mine at right edge of the tunnel */
    me->last_mine_y = 0;

    /* set the tunnel properties... */
    me->wall_thickness_top = 0;
    me->wall_thickness_bottom = 0;
    me->minimal_gap = GAME_SCREEN_HEIGHT - 3;

    /* erase the tunnel walls */
    memset(l_walls, (uint8_t)0,
           (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));


    QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
    me->blink_ctr = 20*2; /* 20s timeout total */
    return QM_ENTRY(&Tunnel_demo_s);
}
/* ${AOs::Tunnel::SM::active::demo} */
static QState Tunnel_demo_x(Tunnel * const me) {
    QActive_disarmX((QActive *)me, 0U);
    return QM_EXIT(&Tunnel_demo_s);
}
/* ${AOs::Tunnel::SM::active::demo} */
static QState Tunnel_demo(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::demo::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            --me->blink_ctr;
            QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
            /* ${AOs::Tunnel::SM::active::demo::Q_TIMEOUT::[me->blink_ctr~]} */
            if (me->blink_ctr == 0) {
                static struct {
                    QMState const *target;
                    QActionHandler act[3];
                } const tatbl_ = { /* transition-action table */
                    &Tunnel_screen_saver_s, /* target state */
                    {
                        Q_ACTION_CAST(&Tunnel_demo_x), /* exit */
                        Q_ACTION_CAST(&Tunnel_screen_saver_i), /* init.tran. */
                        Q_ACTION_CAST(0) /* zero terminator */
                    }
                };
                status_ = QM_TRAN(&tatbl_);
            }
            else {
                status_ = QM_UNHANDLED();
            }
            break;
        }
        /* ${AOs::Tunnel::SM::active::demo::TIME_TICK} */
        case TIME_TICK_SIG: {
            Tunnel_advance(me);
            if ((me->blink_ctr & 1) != 0) {
                /* add the text bitmap into the frame buffer */
                Tunnel_addImageAt(me,
                    PRESS_BUTTON_BMP,
                    (GAME_SCREEN_WIDTH - 55)/2,
                    (GAME_SCREEN_HEIGHT - 8)/2);
            }
            BSP_drawBitmap(l_frame);
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::demo::PLAYER_TRIGGER} */
        case PLAYER_TRIGGER_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[3];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_playing_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_demo_x), /* exit */
                    Q_ACTION_CAST(&Tunnel_playing_e), /* entry */
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
/*${AOs::Tunnel::SM::active::playing} ......................................*/
/* ${AOs::Tunnel::SM::active::playing} */
static QState Tunnel_playing_e(Tunnel * const me) {
    me->minimal_gap = GAME_SCREEN_HEIGHT - 3;

    /* erase the walls */
    memset(l_walls, (uint8_t)0,
           (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));

    QACTIVE_POST((QActive *)&AO_Ship, TAKE_OFF_SIG, 0);
    return QM_ENTRY(&Tunnel_playing_s);
}
/* ${AOs::Tunnel::SM::active::playing} */
static QState Tunnel_playing_x(Tunnel * const me) {
    Tunnel_dispatchToAllMines(me, MINE_RECYCLE_SIG, 0);
    return QM_EXIT(&Tunnel_playing_s);
}
/* ${AOs::Tunnel::SM::active::playing} */
static QState Tunnel_playing(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::playing::TIME_TICK} */
        case TIME_TICK_SIG: {
            BSP_drawBitmap(l_frame); /* render this frame on the display */
            Tunnel_advance(me);
            Tunnel_plantMine(me);
            Tunnel_dispatchToAllMines(me, Q_SIG(me), Q_PAR(me));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::SHIP_IMG} */
        case SHIP_IMG_SIG: {
            uint8_t x   = (uint8_t)Q_PAR(me);
            uint8_t y   = (uint8_t)(Q_PAR(me) >> 8);
            uint8_t bmp = (uint8_t)(Q_PAR(me) >> 16);

            /* did the Ship/Missile hit the tunnel wall? */
            if (Tunnel_isWallHit(me, bmp, x, y)) {
                QACTIVE_POST((QActive *)&AO_Ship, HIT_WALL_SIG, 0);
            }
            Tunnel_addImageAt(me, bmp, x, y);
            Tunnel_dispatchToAllMines(me, Q_SIG(me), Q_PAR(me));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::MISSILE_IMG} */
        case MISSILE_IMG_SIG: {
            uint8_t x   = (uint8_t)Q_PAR(me);
            uint8_t y   = (uint8_t)(Q_PAR(me) >> 8);
            uint8_t bmp = (uint8_t)(Q_PAR(me) >> 16);

            /* did the Ship/Missile hit the tunnel wall? */
            if (Tunnel_isWallHit(me, bmp, x, y)) {
                QACTIVE_POST((QActive *)&AO_Missile, HIT_WALL_SIG, 0);
            }
            Tunnel_addImageAt(me, bmp, x, y);
            Tunnel_dispatchToAllMines(me, Q_SIG(me), Q_PAR(me));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::MINE_IMG} */
        case MINE_IMG_SIG: {
            Tunnel_addImageAt(me,
                              (uint8_t)(Q_PAR(me) >> 16),
                              (uint8_t)Q_PAR(me),
                              (int8_t) (Q_PAR(me) >> 8));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::EXPLOSION} */
        case EXPLOSION_SIG: {
            Tunnel_addImageAt(me,
                              (uint8_t)(Q_PAR(me) >> 16),
                              (uint8_t)Q_PAR(me),
                              (int8_t) (Q_PAR(me) >> 8));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::SCORE} */
        case SCORE_SIG: {
            BSP_updateScore((uint16_t)Q_PAR(me));

            /* increase difficulty of the game:
            * the tunnel gets narrower as the score goes up
            */
            me->minimal_gap = (uint8_t)(GAME_SCREEN_HEIGHT - 3U
                                        - ((uint16_t)Q_PAR(me)/2000U));
            status_ = QM_HANDLED();
            break;
        }
        /* ${AOs::Tunnel::SM::active::playing::GAME_OVER} */
        case GAME_OVER_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[3];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_game_over_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_playing_x), /* exit */
                    Q_ACTION_CAST(&Tunnel_game_over_e), /* entry */
                    Q_ACTION_CAST(0) /* zero terminator */
                }
            };
            uint16_t score = (uint16_t)Q_PAR(me);
            char str[5];

            BSP_updateScore(score);

            /* clear the screen */
            memset(l_frame, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
            BSP_drawBitmap(l_frame); /* render this frame on the display */

            /* Output the final score to the screen */
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*10)/2, 1, "Score:");
            str[4] = '\0';                     /* zero-terminate the string */
            str[3] = '0' + (score % 10); score /= 10;
            str[2] = '0' + (score % 10); score /= 10;
            str[1] = '0' + (score % 10); score /= 10;
            str[0] = '0' + (score % 10);
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*10)/2 + 6*6, 1, str);
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
/*${AOs::Tunnel::SM::active::screen_saver} .................................*/
/* ${AOs::Tunnel::SM::active::screen_saver::initial} */
static QState Tunnel_screen_saver_i(Tunnel * const me) {
    static struct {
        QMState const *target;
        QActionHandler act[2];
    } const tatbl_ = { /* transition-action table */
        &Tunnel_screen_saver_hide_s, /* target state */
        {
            Q_ACTION_CAST(&Tunnel_screen_saver_hide_e), /* entry */
            Q_ACTION_CAST(0) /* zero terminator */
        }
    };
    /* ${AOs::Tunnel::SM::active::screen_saver::initial} */
    return QM_TRAN_INIT(&tatbl_);
}
/* ${AOs::Tunnel::SM::active::screen_saver} */
static QState Tunnel_screen_saver(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::screen_saver::PLAYER_TRIGGER} */
        case PLAYER_TRIGGER_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[2];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_demo_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_demo_e), /* entry */
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
/*${AOs::Tunnel::SM::active::screen_saver::screen_saver_hide} ..............*/
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_hide} */
static QState Tunnel_screen_saver_hide_e(Tunnel * const me) {
    BSP_displayOff();  /* power down the display */
    QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC*3U); /* 3 sec */
    return QM_ENTRY(&Tunnel_screen_saver_hide_s);
}
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_hide} */
static QState Tunnel_screen_saver_hide_x(Tunnel * const me) {
    QActive_disarmX((QActive *)me, 0U);
    BSP_displayOn(); /* power up the display */
    return QM_EXIT(&Tunnel_screen_saver_hide_s);
}
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_hide} */
static QState Tunnel_screen_saver_hide(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_hide::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[3];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_screen_saver_show_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_screen_saver_hide_x), /* exit */
                    Q_ACTION_CAST(&Tunnel_screen_saver_show_e), /* entry */
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
/*${AOs::Tunnel::SM::active::screen_saver::screen_saver_show} ..............*/
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_show} */
static QState Tunnel_screen_saver_show_e(Tunnel * const me) {
    uint32_t rnd = random();
    /* clear the screen frame buffer */
    memset(l_frame, (uint8_t)0,
           (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
    Tunnel_addImageAt(me,
            PRESS_BUTTON_BMP,
            (uint8_t)(rnd % (GAME_SCREEN_WIDTH - 55)),
            (int8_t) (rnd % (GAME_SCREEN_HEIGHT - 8)));
    BSP_drawBitmap(l_frame); /* render this frame on the display */
    QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/3U);/* 1/3 sec */
    return QM_ENTRY(&Tunnel_screen_saver_show_s);
}
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_show} */
static QState Tunnel_screen_saver_show_x(Tunnel * const me) {
    QActive_disarmX((QActive *)me, 0U);
    /* clear the screen frame buffer */
    memset(l_frame, (uint8_t)0,
           (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
    BSP_drawBitmap(l_frame); /* render this frame on the display */
    return QM_EXIT(&Tunnel_screen_saver_show_s);
}
/* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_show} */
static QState Tunnel_screen_saver_show(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::screen_saver::screen_saver_show::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            static struct {
                QMState const *target;
                QActionHandler act[3];
            } const tatbl_ = { /* transition-action table */
                &Tunnel_screen_saver_hide_s, /* target state */
                {
                    Q_ACTION_CAST(&Tunnel_screen_saver_show_x), /* exit */
                    Q_ACTION_CAST(&Tunnel_screen_saver_hide_e), /* entry */
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
/*${AOs::Tunnel::SM::active::game_over} ....................................*/
/* ${AOs::Tunnel::SM::active::game_over} */
static QState Tunnel_game_over_e(Tunnel * const me) {
    QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
    me->blink_ctr = 5U*2U; /* 5s timeout */
    BSP_drawNString((GAME_SCREEN_WIDTH - 6*9)/2, 0, "Game Over");
    return QM_ENTRY(&Tunnel_game_over_s);
}
/* ${AOs::Tunnel::SM::active::game_over} */
static QState Tunnel_game_over_x(Tunnel * const me) {
    QActive_disarmX((QActive *)me, 0U);
    BSP_updateScore(0); /* update the score on the display */
    return QM_EXIT(&Tunnel_game_over_s);
}
/* ${AOs::Tunnel::SM::active::game_over} */
static QState Tunnel_game_over(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::game_over::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            --me->blink_ctr;
            QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*9)/2, 0,
                            (((me->blink_ctr & 1) != 0)
                             ? "Game Over"
                             : "         "));
            /* ${AOs::Tunnel::SM::active::game_over::Q_TIMEOUT::[me->blink_ctr~]} */
            if (me->blink_ctr == 0) {
                static struct {
                    QMState const *target;
                    QActionHandler act[3];
                } const tatbl_ = { /* transition-action table */
                    &Tunnel_demo_s, /* target state */
                    {
                        Q_ACTION_CAST(&Tunnel_game_over_x), /* exit */
                        Q_ACTION_CAST(&Tunnel_demo_e), /* entry */
                        Q_ACTION_CAST(0) /* zero terminator */
                    }
                };
                status_ = QM_TRAN(&tatbl_);
            }
            else {
                status_ = QM_UNHANDLED();
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
/*${AOs::Tunnel::SM::active::show_logo} ....................................*/
/* ${AOs::Tunnel::SM::active::show_logo} */
static QState Tunnel_show_logo_e(Tunnel * const me) {
    QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
    me->blink_ctr = 10U;
    BSP_drawNString(0U, 0U, " Quantum  LeAps ");
    BSP_drawNString(0U, 1U, "state-machine.co");
    return QM_ENTRY(&Tunnel_show_logo_s);
}
/* ${AOs::Tunnel::SM::active::show_logo} */
static QState Tunnel_show_logo_x(Tunnel * const me) {
    QActive_disarmX((QActive *)me, 0U);
    return QM_EXIT(&Tunnel_show_logo_s);
}
/* ${AOs::Tunnel::SM::active::show_logo} */
static QState Tunnel_show_logo(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::Tunnel::SM::active::show_logo::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            --me->blink_ctr; /* toggle the blink couner */
            /* ${AOs::Tunnel::SM::active::show_logo::Q_TIMEOUT::[me->blink_ctr~]} */
            if (me->blink_ctr == 0U) {
                static struct {
                    QMState const *target;
                    QActionHandler act[3];
                } const tatbl_ = { /* transition-action table */
                    &Tunnel_demo_s, /* target state */
                    {
                        Q_ACTION_CAST(&Tunnel_show_logo_x), /* exit */
                        Q_ACTION_CAST(&Tunnel_demo_e), /* entry */
                        Q_ACTION_CAST(0) /* zero terminator */
                    }
                };
                status_ = QM_TRAN(&tatbl_);
            }
            /* ${AOs::Tunnel::SM::active::show_logo::Q_TIMEOUT::[else]} */
            else {
                QActive_armX((QActive *)me, 0U, BSP_TICKS_PER_SEC/2U); /* 1/2 sec */
                /* ${AOs::Tunnel::SM::active::show_logo::Q_TIMEOUT::[else]::[(me->blink_ct~]} */
                if ((me->blink_ctr & 1U) != 0U) {
                    BSP_drawNString(6U*9U, 0U,         " LeAps");
                    BSP_drawNString(0U,    1U, "state-machine.co");
                    status_ = QM_HANDLED();
                }
                /* ${AOs::Tunnel::SM::active::show_logo::Q_TIMEOUT::[else]::[else]} */
                else {
                    BSP_drawNString(6U*9U, 0U,         "LeaPs ");
                    BSP_drawNString(0U,    1U, "tate-machine.com");
                    status_ = QM_HANDLED();
                }
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
/*${AOs::Tunnel::SM::final} ................................................*/
/* ${AOs::Tunnel::SM::final} */
static QState Tunnel_final_e(Tunnel * const me) {
    /* clear the screen */
    memset(l_frame, (uint8_t)0,
           (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
    BSP_drawBitmap(l_frame); /* render this frame on the display */
    QF_stop();  /* stop QF and cleanup */
    (void)me; /* avoid compiler warning in case 'me' is not used */
    return QM_ENTRY(&Tunnel_final_s);
}
/* ${AOs::Tunnel::SM::final} */
static QState Tunnel_final(Tunnel * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    (void)me; /* avoid compiler warning in case 'me' is not used */
    return status_;
}


/*..........................................................................*/
uint32_t random(void) {      /* a very cheap pseudo-random-number generator */
    /* "Super-Duper" Linear Congruential Generator (LCG)
    * LCG(2^32, 3*7*11*13*23, 0, seed)
    */
    l_rnd = l_rnd * (3*7*11*13*23);
    return l_rnd >> 8;
}
/*..........................................................................*/
void randomSeed(uint32_t seed) {
    l_rnd = seed;
}
/*..........................................................................*/
uint8_t do_bitmaps_overlap(uint8_t bmp_id1, uint8_t x1, uint8_t y1,
                           uint8_t bmp_id2, uint8_t x2, uint8_t y2)
{
    uint8_t x;
    uint8_t x0;
    uint8_t w;
    uint32_t bits1;
    uint32_t bits2;
    Bitmap const *bmp1;
    Bitmap const *bmp2;

    Q_REQUIRE((bmp_id1 < Q_DIM(l_bitmap)) && (bmp_id2 < Q_DIM(l_bitmap)));

    bmp1 = &l_bitmap[bmp_id1];
    bmp2 = &l_bitmap[bmp_id2];

             /* is the incoming object starting to overlap the Mine bitmap? */
    if ((x1 <= x2) && (x1 + bmp2->width > x2)) {
        x0 = x2 - x1;
        w  = x1 + bmp2->width - x2;
        if (w > bmp1->width) {
            w = bmp1->width;
        }
        for (x = 0; x < w; ++x) {      /* scan over the overlapping columns */
            bits1 = ((uint32_t)bmp2->bits[x + x0] << y2);
            bits2 = ((uint32_t)bmp1->bits[x] << y1);
            if ((bits1 & bits2) != 0) {             /* do the bits overlap? */
                return (uint8_t)1;                                  /* yes! */
            }
        }
    }
    else {
        if ((x1 > x2) && (x2 + bmp1->width > x1)) {
            x0 = x1 - x2;
            w  = x2 + bmp1->width - x1;
            if (w > bmp2->width) {
                w = bmp2->width;
            }
            for (x = 0; x < w; ++x) {  /* scan over the overlapping columns */
                bits1 = ((uint32_t)bmp1->bits[x + x0] << y1);
                bits2 = ((uint32_t)bmp2->bits[x] << y2);
                if ((bits1 & bits2) != 0) {         /* do the bits overlap? */
                    return (uint8_t)1;                              /* yes! */
                }
            }
        }
    }
    return (uint8_t)0;                        /* the bitmaps do not overlap */
}
