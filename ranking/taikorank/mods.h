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

// --------------- MODS COEFF --------------- 
#define EZ_COEFF_OD    0.5
#define EZ_COEFF_SPEED (2 / 3.)

#define HR_COEFF_OD    1.4
#define HR_COEFF_SPEED (4 / 3.)

#define DT_COEFF_MS    (2 / 3.)
#define DT_COEFF_SPEED 1.5
#define DT_COEFF_BPM   1.5

#define HT_COEFF_MS    (4 / 3.)
#define HT_COEFF_SPEED (2 / 3.)
#define HT_COEFF_BPM   (2 / 3.)

#define HD_NB_OBJ_APP  10.
#define HD_NB_OBJ_DIS  7.

#define FL_NB_OBJ_APP1 5.
#define FL_NB_OBJ_APP2 4.
#define FL_NB_OBJ_APP3 3.5
#define FL_START_APP2  100 // if >=
#define FL_START_APP3  200 // if >=

// --------------- NO MODS COEFF --------------- 
#define NM_NB_OBJ_APP  17.
#define NM_NB_OBJ_DIS  0.

#define NM_COEFF_MS    1

#define MAX_OD 10.

// -----------------------------

void trm_apply_mods (struct tr_map * map);

#endif //MODS_H
