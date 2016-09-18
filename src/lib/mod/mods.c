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
#include <stdio.h>
#include <stdint.h>
#include "osux/mods.h"

struct mod_designation {
    const char *long_;
    const char *short_;
};

static struct mod_designation none = { "None",        "None" };

static struct mod_designation md[] = {
    [ MOD_ID_NOFAIL ]       = { "NoFail",      "NF" },
    [ MOD_ID_EASY ]         = { "Easy",        "EZ" },
    [ MOD_ID_NOVIDEO ]      = { "NoVideo",     "" },
    [ MOD_ID_HIDDEN ]       = { "Hidden",      "HD" },
    [ MOD_ID_HARDROCK ]     = { "HardRock",    "HR" },
    [ MOD_ID_SUDDENDEATH ]  = { "SuddenDeath", "SD" },
    [ MOD_ID_DOUBLETIME ]   = { "DoubleTime",  "DT" },
    [ MOD_ID_RELAX ]        = { "Relax",       "Relax" },
    [ MOD_ID_HALFTIME ]     = { "HalfTime",    "HT" },
    [ MOD_ID_NIGHTCORE ]    = { "NightCore",   "NC" },
    [ MOD_ID_FLASHLIGHT ]   = { "FlashLight",  "FL" },
    [ MOD_ID_AUTOPLAY ]     = { "Auto",        "Auto" },
    [ MOD_ID_SPUNOUT ]      = { "SpunOut",     "SO" },
    [ MOD_ID_AUTOPILOT ]    = { "AutoPilot",   "AP" },
    [ MOD_ID_PERFECT ]      = { "Perfect",     "PF" },
    [ MOD_ID_1K ]           = { "Key1",        "1K" },
    [ MOD_ID_2K ]           = { "Key2",        "2K" },
    [ MOD_ID_3K ]           = { "Key3",        "3K" },
    [ MOD_ID_4K ]           = { "Key4",        "4K" },
    [ MOD_ID_5K ]           = { "Key5",        "5K" },
    [ MOD_ID_6K ]           = { "Key6",        "6K" },
    [ MOD_ID_7K ]           = { "Key7",        "7K" },
    [ MOD_ID_8K ]           = { "Key8",        "8K" },
    [ MOD_ID_9K ]           = { "Key9",        "9K" },
    [ MOD_ID_FADEIN ]       = { "FadeIn",      "FI" },
    [ MOD_ID_RANDOM ]       = { "Random",      "RD" },
    [ MOD_ID_CINEMA ]       = { "Cinema",      "CN" },
    [ MOD_ID_COOP ]         = { "Coop",        "CP" },
    [ MOD_ID_TARGETPRACTICE ] = { "TargetPractice", "TP" },
};

void mod_print(FILE *f, uint32_t mod_bitfield)
{
    unsigned count = 0;

    if (mod_bitfield == 0) {
	fputs(none.short_, f);
	return;
    }

    for (int i = 0; i < MOD_ID_MAX; ++i) {
	if ((mod_bitfield >> i) % 2) {
	    if (count) fprintf(f, "|");
	    fputs(md[i].short_, f);
	    ++ count;
	}
    }
}
