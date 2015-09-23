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
#include "util/hashtable/hashtable.h"

#include "hs/hitsound.h"
#include "map/map.h"
#include "combocolor/combocolor.h"
#include "ho/hit_object.h"
#include "tp/timing_point.h"

#include "omp.h"
#include "re.h"

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

static void event_subsection_parser(char *line, struct generic_entry *ge)
{

}

static void event_parser(char *line, struct generic_entry *ge)
{
    
}

#define line_is_empty(line)						\
    (line == NULL || *line == '\0' || *line == '\r' || *line == '\n')	\

#define line_is_comment(line)					\
    (line != NULL && (*line == '/' || *(line+1) == '/'))	\


__internal
struct hash_table *omp_c_parse_osu_file(FILE *f, int version)
{
    struct hash_table *sections;
    static regex_t section_delim;
    struct list *hit_objects;
    struct list *timing_points;
    char *current_section_name = NULL;
    struct hash_table *current_section = NULL;
    
    sections = ht_create(NULL);
    hit_objects = list_new(0);
    timing_points = list_new(0);
    
    ht_add_entry(sections, "HitObjects", hit_objects);
    ht_add_entry(sections, "TimingPoints", timing_points);

    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, f) != NULL) {
	char *match[1];
	
	if (re_match("^\\[(.*)\\]", line, 1, match) == 0) {
	    free(current_section_name);
	    current_section_name = match[0];
	    if (!ht_has_entry(sections, current_section_name)) {
		current_section = ht_create(NULL);
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
	    // parse subsection and event in a similar manner
	    // ev_c_parse() ...
	} else if (!strcmp(current_section_name, "HitObjects")) {
	    struct hit_object *ho = calloc(sizeof(*ho), 1);
	    ho_c_parse(line, ho, version);
	    list_add(hit_objects, ho);
	} else if (!strcmp(current_section_name, "TimingPoints")) {
	    struct timing_point *tp = malloc(sizeof(*tp));
	    tp_c_parse(line, tp);
	    list_add(timing_points, tp);
	} else {
	    struct h_entry e;
	    default_parser(line, &e);
	    ht_add_entry(current_section, e.id, e.value);
	    free(e.id);
	}
    }

    free(current_section);
    regfree(&section_delim);
    return sections;
}
