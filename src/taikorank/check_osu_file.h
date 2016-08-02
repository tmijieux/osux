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

#ifndef TR_LOAD_OSU_H
#define TR_LOAD_OSU_H

enum tr_load_osu {
    TR_FILENAME_ERROR = 0,
    TR_FILENAME_OSU_FILE = 1,
    TR_FILENAME_HASH = 2,
};

int tr_check_file(char *file_name);

#endif // TR_LOAD_OSU_H
