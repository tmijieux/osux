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

#include "mods.h"

struct mod_designation {
    const char *long_;
    const char *short_;
    
};

struct mod_designation mods_designation[] = {
    [ IMOD_NONE ]         = { "None",        "" },
    [ IMOD_NOFAIL ]       = { "NoFail",      "NF" },
    [ IMOD_EASY ]         = { "Easy",        "EZ" },
    [ IMOD_NOVIDEO ]      = { "NoVideo",     "" },
    [ IMOD_HIDDEN ]       = { "Hidden",      "HD" },
    [ IMOD_HARDROCK ]     = { "HardRock",    "HR" },
    [ IMOD_SUDDENDEATH ]  = { "SuddenDeath", "SD" },
    [ IMOD_DOUBLETIME ]   = { "DoubleTime",  "DT" },
    [ IMOD_RELAX ]        = { "Relax",       "Relax" },
    [ IMOD_HALFTIME ]     = { "HalfTime",    "HT" },
    [ IMOD_NIGHTCORE ]    = { "NightCore",   "NC" },
    [ IMOD_FLASHLIGHT ]   = { "FlashLight",  "FL" },
    [ IMOD_AUTOPLAY ]     = { "Auto",        "Auto" },
    [ IMOD_SPUNOUT ]      = { "SpunOut",     "SO" },
    [ IMOD_RELAX2 ]       = { "AutoPilot",   "AP" },
    [ IMOD_PERFECT ]      = { "Perfect",     "PF" },
    [ IMOD_1K ]           = { "Key1",        "1K" },
    [ IMOD_2K ]           = { "Key2",        "2K" },
    [ IMOD_3K ]           = { "Key3",        "3K" },
    [ IMOD_4K ]           = { "Key4",        "4K" },
    [ IMOD_5K ]           = { "Key5",        "5K" },
    [ IMOD_6K ]           = { "Key6",        "6K" },
    [ IMOD_7K ]           = { "Key7",        "7K" },
    [ IMOD_8K ]           = { "Key8",        "8K" },
    [ IMOD_9K ]           = { "Key9",        "9K" },
    [ IMOD_FADEIN ]       = { "FadeIn",      "FI" },
    [ IMOD_RANDOM ]       = { "Random",      "RD" },
    [ IMOD_LASTMOD ]      = { "Cinema",      "" },
    

}
    
