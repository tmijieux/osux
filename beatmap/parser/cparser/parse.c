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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <string.h>

#include <visibility.h>

#include "util/list/list.h"
#include "util/hashtable/hash_table.h"

#include "beatmap/beatmap.h"
#include "beatmap/hitobject.h"
#include "beatmap/timingpoint.h"
#include "beatmap/hitsound.h"

#include "parse.h"
#include "re.h"


/******************************************************************************/
/* OMP
 */

#define line_is_empty(line)						\
    (line == NULL || *line == '\0' || *line == '\r' || *line == '\n')	\

#define line_is_comment(line)					\
    (line != NULL && (*line == '/' || *(line+1) == '/'))	\

#define LINE_SIZE  8096

struct h_entry {
    char *id;
    char *value;
};

static void default_parser(char *line, struct h_entry *e)
{
    char *matches[2];
    if (re_match("^ *([^ ]*) *: *(.*) *$", line, 2, matches) == 0) {
	e->id = matches[0];
	e->value = matches[1];
    }
}

static struct hash_table *event_parse(char *line)
{
    return NULL;
}

static struct hash_table *col_c_parse(char *line)
{
    return NULL;
}

static struct hash_table *tp_c_parse(char *line)
{
    return NULL;
}

static struct hash_table *ho_c_parse(char *line)
{
    return NULL;
}


static void check_format(FILE *f, int *v, int *bbom)
{
    unsigned char bom[3];
    fread(bom, 3, 1, f);
    *bbom = 1;
    if ( !(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
	rewind(f);
	*bbom = 0;
    }
    
    *v = -1;
    if (fscanf(f, u8"osu file format v%d\r\n", v) != 1) {
	return;
    }
}

__internal
struct hash_table *cparse_osu_file(FILE *f)
{
    struct hash_table *sections;

    int *ver = malloc(sizeof(*ver));   // TODO THINK TO FREE
    int *bom = malloc(sizeof(*bom));   // idem
    
    check_format(f, ver, bom);
    if (*ver < 0) {
	free(ver); free(bom);
	return NULL;
    }
    
    sections = ht_create(0, NULL);
    ht_add_entry(sections, "version", ver);
    ht_add_entry(sections, "bom", bom);
    
    struct list *hit_objects = list_new(0);
    struct list *timing_points = list_new(0);
    struct list *colors = list_new(0);
    struct list *events = list_new(0);
    ht_add_entry(sections, "HitObjects", hit_objects);
    ht_add_entry(sections, "TimingPoints", timing_points);
    ht_add_entry(sections, "Colours", colors);
    ht_add_entry(sections, "Events", events);

    struct hash_table *current_section = NULL;
    char *current_section_name = NULL;
    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, f) != NULL) {
	char *match[1];
	
	if (re_match("^\\[(.*)\\]", line, 1, match) == 0) {
	    free(current_section_name);
	    current_section_name = match[0];
	    if (!ht_has_entry(sections, current_section_name)) {
		current_section = ht_create(0, NULL);
		ht_add_entry(sections, current_section_name, current_section);
	    } else {
		ht_get_entry(sections, current_section_name, &current_section);
	    }
	    continue;
	}

	if (!current_section_name ||
	    line_is_empty(line) ||   line_is_comment(line))
	    continue;
	
	if (!strcmp(current_section_name, "Events")) {
	    struct hash_table *ev = event_parse(line);
	    if (ev)
		list_add(events, ev);
	} else if (!strcmp(current_section_name, "HitObjects")) {
	    struct hash_table *ho = ho_c_parse(line);
	    list_add(hit_objects, ho);
	} else if (!strcmp(current_section_name, "TimingPoints")) {
	    struct hash_table *tp = tp_c_parse(line);
	    list_add(timing_points, tp);
	} else if (!strcmp(current_section_name, "Colours")) {
	    struct hash_table *col = col_c_parse(line);
	    list_add(colors, col);
	} else {
	    struct h_entry e;
	    default_parser(line, &e);
	    ht_add_entry(current_section, e.id, e.value);
	    free(e.id);
	}
    }

    free(current_section);
    return sections;
}


