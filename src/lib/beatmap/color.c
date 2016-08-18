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

#include <stdio.h>
#include <glib.h>
#include "osux/color.h"
#include "osux/error.h"
#include "osux/util.h"

int color_parse_data(osux_color *c, char *id, char *data)
{
    char **split;
    c->id = g_ascii_strtoull(id, NULL, 10);
    split = g_strsplit(data, ",", 0);
    unsigned size = strsplit_size(split);
    if (size != 3) {
        g_strfreev(split);
        return -OSUX_ERR_INVALID_COLOR;
    }

    c->r = atoi(split[0]);
    c->g = atoi(split[1]);
    c->b = atoi(split[2]);
    c->a = 255;
    
    g_strfreev(split);
    return 0;
}

int osux_color_init(osux_color *c, char *line, uint32_t osu_version)
{
    int err = 0;
    static GRegex *regexp = NULL;
    char *id, *data;
    
    if (regexp == NULL)
        regexp = g_regex_new("^Color([0-9]+) ?: (.*)$", 0, 0, NULL);

    (void) osu_version;
    GMatchInfo *info = NULL;
    if (!g_regex_match(regexp, line, 0, &info)) {
        g_match_info_free(info);
        return -OSUX_ERR_INVALID_COLOR;
    }

    if (g_match_info_matches(info)) {
        id = g_match_info_fetch(info, 1);
        data = g_match_info_fetch(info, 2);
        err = color_parse_data(c, id, data);
        g_free(data);
    }
    g_match_info_free(info);
    return err;
}

void osux_color_free(osux_color *c)
{
    (void) c;
}

void osux_color_print(FILE *f, osux_color *c)
{
    fprintf(f, "Combo%d : %d,%d,%d\r\n", c->id, c->r, c->g, c->b);
}
