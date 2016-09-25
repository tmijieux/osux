#ifndef OSUX_MODS_H
#define OSUX_MODS_H

/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <glib.h>
#include <stdio.h>
#include <stdint.h>

G_BEGIN_DECLS

void mod_print(FILE *f, uint32_t mod_bitfield);

enum game_mods_id {
    MOD_ID_NOFAIL       = 0,
    MOD_ID_EASY         = 1,
    MOD_ID_NOVIDEO      = 2,
    MOD_ID_HIDDEN       = 3,
    MOD_ID_HARDROCK     = 4,
    MOD_ID_SUDDENDEATH  = 5,
    MOD_ID_DOUBLETIME   = 6,
    MOD_ID_RELAX        = 7,
    MOD_ID_HALFTIME     = 8,
    MOD_ID_NIGHTCORE    = 9,
    MOD_ID_FLASHLIGHT   = 10,

    MOD_ID_AUTOPLAY     = 11,
    MOD_ID_SPUNOUT      = 12,
    MOD_ID_AUTOPILOT    = 13,
    MOD_ID_PERFECT      = 14,

    MOD_ID_4K           = 15,
    MOD_ID_5K           = 16,
    MOD_ID_6K           = 17,
    MOD_ID_7K           = 18,
    MOD_ID_8K           = 19,

    MOD_ID_FADEIN       = 20,
    MOD_ID_RANDOM       = 21,
    MOD_ID_CINEMA       = 22,

    MOD_ID_TARGETPRACTICE = 23,

    MOD_ID_9K           = 24,
    MOD_ID_COOP         = 25,

    MOD_ID_1K           = 26,
    MOD_ID_3K           = 27,
    MOD_ID_2K           = 28,
    MOD_ID_MAX,
};

enum game_mods {
    MOD_NOMOD           =        0,
    MOD_NOFAIL          =       (1 << MOD_ID_NOFAIL),
    MOD_EASY            =       (1 << MOD_ID_EASY),
    MOD_NOVIDEO         =       (1 << MOD_ID_NOVIDEO),
    MOD_HIDDEN          =       (1 << MOD_ID_HIDDEN),
    MOD_HARDROCK        =       (1 << MOD_ID_HARDROCK),
    MOD_SUDDENDEATH     =       (1 << MOD_ID_SUDDENDEATH),
    MOD_DOUBLETIME      =       (1 << MOD_ID_DOUBLETIME),
    MOD_RELAX           =       (1 << MOD_ID_RELAX),
    MOD_HALFTIME        =       (1 << MOD_ID_HALFTIME),
    MOD_NIGHTCORE       =       (1 << MOD_ID_NIGHTCORE),
    // never set without MOD_DOUBLETIME

    MOD_FLASHLIGHT      =       (1 << MOD_ID_FLASHLIGHT),
    MOD_AUTOPLAY        =       (1 << MOD_ID_AUTOPLAY),
    MOD_SPUNOUT         =       (1 << MOD_ID_SPUNOUT),
    MOD_AUTOPILOT       =       (1 << MOD_ID_AUTOPILOT),
    MOD_PERFECT         =       (1 << MOD_ID_PERFECT),

    MOD_4K              =       (1 << MOD_ID_4K),
    MOD_5K              =       (1 << MOD_ID_5K),
    MOD_6K              =       (1 << MOD_ID_6K),
    MOD_7K              =       (1 << MOD_ID_7K),
    MOD_8K              =       (1 << MOD_ID_8K),

    MOD_FADEIN          =       (1 << MOD_ID_FADEIN),
    MOD_RANDOM          =       (1 << MOD_ID_RANDOM),
    MOD_CINEMA          =       (1 << MOD_ID_CINEMA),

    MOD_TARGETPRACTICE  =       (1 << MOD_ID_TARGETPRACTICE),

    MOD_9K              =       (1 << MOD_ID_9K),
    MOD_COOP            =       (1 << MOD_ID_COOP),

    MOD_1K              =       (1 << MOD_ID_1K),
    MOD_3K              =       (1 << MOD_ID_3K),
    MOD_2K              =       (1 << MOD_ID_2K),

    MOD_1K_COOP         =       MOD_2K,
    MOD_2K_COOP         =       MOD_4K,
    MOD_3K_COOP         =       MOD_6K,
    MOD_4K_COOP         =       MOD_8K,

    MOD_5K_COOP         =       (MOD_5K | MOD_COOP),
    MOD_6K_COOP         =       (MOD_6K | MOD_COOP),
    MOD_7K_COOP         =       (MOD_7K | MOD_COOP),
    MOD_8K_COOP         =       (MOD_8K | MOD_COOP),
    MOD_9K_COOP         =       (MOD_9K | MOD_COOP)

};


G_END_DECLS

#endif // OSUX_MODS_H
