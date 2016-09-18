#ifndef OSUX_HITSOUND_H
#define OSUX_HITSOUND_H

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
#include <glib/gi18n.h>

G_BEGIN_DECLS

enum hitsound_sample {
    SAMPLE_NORMAL  = 0x00,
    SAMPLE_UNK1    = 0x01,
    SAMPLE_WHISTLE = 0x02,
    SAMPLE_FINISH  = 0x04,
    SAMPLE_CLAP    = 0x08,

    SAMPLE_TAIKO_DON = SAMPLE_NORMAL,
    SAMPLE_TAIKO_KAT = SAMPLE_CLAP | SAMPLE_WHISTLE,
    SAMPLE_TAIKO_BIG = SAMPLE_FINISH,
};

#define SAMPLE_SETS(SET)                        \
    SET(0, DEFAULT, N_("Default"))              \
    SET(1, NORMAL, N_("Normal"))                \
    SET(2, SOFT, N_("Soft"))                    \
    SET(3, DRUM, N_("Drum"))                    \

gchar const *osux_sample_set_get_name(int sample_set);
gchar const *osux_sample_set_get_localized_name(int sample_set);

#define SAMPLE_SET_TO_ENUM_(value_, caps_, pretty_)     \
    SAMPLE_TYPE_##caps_ = (value_),

enum hitsound_sample_type {
    SAMPLE_SETS(SAMPLE_SET_TO_ENUM_)
    MAX_SAMPLE_TYPE,
};

G_END_DECLS

#endif // OSUX_HITSOUND_H
