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
#ifndef HITOBJECT_TAIKO_H
#define HITOBJECT_TAIKO_H

#include "osux/hitobject.h"

// create a simple circle for taiko mode
osux_hitobject *osux_ho_taiko_circle_new(int offset, int sample);

// create a simple spinner for taiko mode
osux_hitobject *osux_ho_taiko_spinner_new(int offset, int end_offset);

#endif //HITOBJECT_TAIKO_H
