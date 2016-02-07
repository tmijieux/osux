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

#include "uleb128.h"

uint64_t read_ULEB128(FILE * f)
{
    uint64_t value = 0;				
    unsigned shift = 0;				
    uint8_t p;					
    do {						
	fread(&p, 1, 1, f);				
	value += (uint64_t)(p & 0x7f) << shift;	
	shift += 7;					
    } while (p >= 0x80);				
    return value;						
}							

void write_ULEB128(uint64_t value, FILE *output, unsigned padding)
{
    do {
        uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value != 0 || padding != 0)
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        fwrite(&byte, 1, 1, output);
    } while (value != 0);
 
    // Pad with 0x80 and emit a null byte at the end.
    if (padding != 0) {
        char v[] = { '\x80', '\0' };
        for (; padding != 1; --padding)
            fwrite(&v[0], 1, 1, output);
        fwrite(&v[1], 1, 1, output);
    }
}
