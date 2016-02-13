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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"

#include "check_osu_file.h"

int check_file(char * file_name)
{
    // cheking that it's a .osu file
    int length = strlen(file_name);
    if (strncmp(".osu", &file_name[length-4], 5) != 0)
	return 2; // that's a hash
    // check that the file existence
    if (access(file_name, F_OK) == -1) {
	tr_error("%s: Please let me open your file :S", file_name);
	return 0;
    }
    return 1;
}
