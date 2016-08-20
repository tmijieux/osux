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
#include <glib/gi18n.h>
#include <glib.h>
#include "osux/color.h"
#include "osux/error.h"
#include "osux/util.h"

#define COLOR_TYPE_TO_STRING(capital_, pretty_) \
    [COLOR_##capital_] = pretty_,

static char const *color_type_str[MAX_COLOR_TYPE] = {
    COLOR_TYPES(COLOR_TYPE_TO_STRING)
};

char const *osux_color_type_get_name(int type)
{
    if (type < 0 || type >= MAX_COLOR_TYPE)
        return NULL;
    return gettext(color_type_str[type]);
}

static int parse_data(osux_color *c, char *type, char *data)
{
    int err = 0;
    char **split;

    split = g_strsplit(data, ",", 0);
    unsigned size = strsplit_size(split);
    if (size == 3) {
        c->r = atoi(split[0]);
        c->g = atoi(split[1]);
        c->b = atoi(split[2]);
        c->a = 255;
    } else
        err = -OSUX_ERR_INVALID_COLOR;

    if (!err && c->type == COLOR_COMBO) {
        if (sscanf(type, "Combo%d", &c->id) != 1)
            err = - OSUX_ERR_INVALID_COLOR;
    }

    g_strfreev(split);
    return err;
}

#define COLOR_PARSE_TYPE(capital_, pretty_)                             \
    do {                                                                \
        if (!g_ascii_strncasecmp(type, pretty_, strlen(pretty_))) {     \
            c->type = COLOR_##capital_;                                 \
            return 0;                                                   \
        }                                                               \
    } while (0);

static int parse_type(osux_color *c, char *type)
{
    COLOR_TYPES(COLOR_PARSE_TYPE);
    osux_warning("invalid color type '%s'\n", type);
    return -OSUX_ERR_INVALID_COLOR_TYPE;
}

int osux_color_init(osux_color *c, char *line, uint32_t osu_version)
{
    int err = 0;
    static GRegex *regexp = NULL;
    char *type, *data;

    if (regexp == NULL)
        regexp = g_regex_new("([^:]*):([^:]*)$", 0, 0, NULL);

    (void) osu_version;
    GMatchInfo *info = NULL;
    if (!g_regex_match(regexp, line, 0, &info)) {
        g_match_info_free(info);
        return -OSUX_ERR_INVALID_COLOR;
    }

    if (g_match_info_matches(info)) {
        type = g_match_info_fetch(info, 1);
        data = g_match_info_fetch(info, 2);
        g_strstrip(type);
        g_strstrip(data);
        err = parse_type(c, type);
        if (!err)
            err = parse_data(c, type, data);
        g_free(type);
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
    if (c->type == COLOR_COMBO)
        fprintf(f, "Combo%d : %d,%d,%d\r\n", c->id, c->r, c->g, c->b);
    else
        fprintf(f, "%s: %d,%d,%d\r\n", color_type_str[c->type], c->r, c->g, c->b);
}
