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
#ifndef BPM_H
#define BPM_H

#define MSEC_IN_MINUTE 60000.

static inline double mpb_to_bpm(double mpb)
{
    return MSEC_IN_MINUTE / mpb;
}

static inline double bpm_to_mpb(double bpm)
{
    return MSEC_IN_MINUTE / bpm;
}

#endif //BPM_H
