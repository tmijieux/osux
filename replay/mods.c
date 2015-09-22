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
    [ MOD_NONE ]         = { "None",        "" },
    [ MOD_NOFAIL ]       = { "NoFail",      "NF" },
    [ MOD_EASY ]         = { "Easy",        "EZ" },
    [ MOD_NOVIDEO ]      = { "NoVideo",     "" },
    [ MOD_HIDDEN ]       = { "Hidden",      "HD" },
    [ MOD_HARDROCK ]     = { "HardRock",    "HR" },
    [ MOD_SUDDENDEATH ]  = { "SuddenDeath", "SD" },
    [ MOD_DOUBLETIME ]   = { "DoubleTime",  "DT" },
    [ MOD_RELAX ]        = { "Relax",       "Relax" },
    [ MOD_HALFTIME ]     = { "HalfTime",    "HT" },
    [ MOD_NIGHTCORE ]    = { "NightCore",   "NC" },
    [ MOD_FLASHLIGHT ]   = { "FlashLight",  "FL" },
    [ MOD_AUTOPLAY ]     = { "Auto",        "Auto" },
    [ MOD_SPUNOUT ]      = { "SpunOut",     "SO" },
    [ MOD_RELAX2 ]       = { "AutoPilot",   "AP" },
    [ MOD_PERFECT ]      = { "Perfect",     "PF" },
    [ MOD_KEY4 ]         = { "Key4",        "4K" },
    [ MOD_KEY5 ]         = { "Key5",        "5K" },
    [ MOD_KEY6 ]         = { "Key6",        "6K" },
    [ MOD_KEY7 ]         = { "Key7",        "7K" },
    [ MOD_KEY8 ]         = { "Key8",        "8K" },
    [ MOD_KEYMOD ]       = { "KeyMod",      "" }, //8+ 7 +6
    [ MOD_FADEIN ]       = { "FadeIn",      "FI" },
    [ MOD_RANDOM ]       = { "Random",      "RD" },
    [ MOD_LASTMOD ]      = { "Cinema",      "" },
    

}
    
