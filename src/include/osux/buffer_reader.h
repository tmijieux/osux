#ifndef OSUX_BUFFER_READER_H
#define OSUX_BUFFER_READER_H

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

#include <stdlib.h>
#include <stdint.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct osux_buffer_reader_ {
    void *data;
    size_t size;
    uint64_t r; // read pointer
} osux_buffer_reader;

int osux_buffer_reader_init(osux_buffer_reader *br, void *data, size_t size);

ssize_t obr_read(osux_buffer_reader *br, void *data, size_t size);
int obr_read_uleb128(osux_buffer_reader *br, uint64_t *value);
int obr_read_string(osux_buffer_reader *br, char **value);
int obr_read_lzma(osux_buffer_reader *br, char **value, size_t size);

int osux_buffer_reader_free(osux_buffer_reader *br);

G_END_DECLS

#endif // OSUX_BUFFER_READER_H
