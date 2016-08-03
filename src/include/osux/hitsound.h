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

enum hitsound_sample {
    SAMPLE_NORMAL  = 0x01,
    SAMPLE_WHISTLE = 0x02,
    SAMPLE_FINISH  = 0x04,
    SAMPLE_CLAP    = 0x08,
};

enum hitsound_sample_type {
    SAMPLE_TYPE_DEFAULT   = 0,  // defaults to timing section sample type.
    SAMPLE_TYPE_NORMAL    = 1,
    SAMPLE_TYPE_SOFT      = 2,
    SAMPLE_TYPE_DRUM      = 3,
};

#endif // OSUX_HITSOUND_H
