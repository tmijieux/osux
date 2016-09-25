/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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
#ifndef TR_MODS_H
#define TR_MODS_H

// --------------- MODS ---------------

#define MOD_NM MOD_NOMOD
#define MOD_EZ MOD_EASY
#define MOD_HT MOD_HALFTIME
#define MOD_HR MOD_HARDROCK
#define MOD_DT MOD_DOUBLETIME
#define MOD_HD MOD_HIDDEN
#define MOD_FL MOD_FLASHLIGHT

#define RESOLUTION_10

// ----------------------------------------

int trm_has_mods(const struct tr_map *map, int mods);

void trm_apply_mods_FL(struct tr_map *map);

void trm_apply_mods(struct tr_map *map);
void trm_print_out_mods(const struct tr_map *map);
char *trm_mods_to_str(const struct tr_map *map);
int str_to_mods(const char *s);

#endif // TR_MODS_H
