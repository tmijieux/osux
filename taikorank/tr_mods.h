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

#ifndef MODS_H
#define MODS_H

// --------------- MODS ---------------
#define MODS_NONE 0
#define MODS_EZ   (1 << 0)
#define MODS_HT   (1 << 1)
#define MODS_HR   (1 << 2)
#define MODS_DT   (1 << 3)
#define MODS_HD   (1 << 4)
#define MODS_FL   (1 << 5)

#define RESOLUTION_10

// ----------------------------------------

void trm_apply_mods_FL(struct tr_map * map);

void trm_apply_mods(struct tr_map * map);
void trm_print_mods(struct tr_map * map);
char * trm_mods_to_str(struct tr_map * map);

#endif //MODS_H
