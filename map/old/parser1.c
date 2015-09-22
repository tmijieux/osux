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

#include "hs/hitsound.h"
#include "list/list.h"
#include "hashtable/hashtable.h"

#include "../map.h"
#include "../color.h"
#include "../ho/hit_object.h"
#include "../timing/timing_point.h"

#include "parser1.h"
#include "regread.h"


#define LINE_SIZE  8096

struct generic_entry {
    char id[LINE_SIZE];
    char value[LINE_SIZE];
    int size;
};

static void default_parser(char *line, struct generic_entry *ge)
{
    static int comp = 0;
    static regex_t r;
    if (!comp) {
	regcomp(&r, "^ *([^ ]*) *: *(.*) *\r$",
		REG_EXTENDED | REG_NEWLINE);
    	comp = 1;
    }

    regmatch_t mtch[3];
    regexec(&r, line, 3, mtch, 0);
    strncpy(ge->id, reg_read_match( line, mtch, 1 ), LINE_SIZE );
    strncpy(ge->value, reg_read_match( line, mtch, 2 ), LINE_SIZE );
    ge->size = strlen(ge->value);
}


static void event_subsection_parser(char *line, struct generic_entry *ge)
{

}

static void event_parser(char *line, struct generic_entry *ge)
{
    
}

__internal
struct hash_table *read_osu_file(FILE *f, int version)
{
    struct hash_table *sections = ht_create(NULL);

    char *current_section = NULL;
    struct hash_table *current_section_ht = NULL;
    
    static regex_t section_delim;
    regcomp(&section_delim, "^\\[(.*)\\]", REG_EXTENDED);
    struct list *hit_objects = list_new(0);
    struct list *timing_points = list_new(0);

    ht_add_entry(sections, "HitObjects", hit_objects);
    ht_add_entry(sections, "TimingPoints", timing_points);
    
    char line[LINE_SIZE];

    while (fgets(line, LINE_SIZE, f) != NULL) {
	regmatch_t m[2];
	
	if (regexec(&section_delim, line, 2, m, 0) != REG_NOMATCH) {
	    free(current_section);
	    line[m[1].rm_eo] = 0;
	    current_section = strdup(&line[m[1].rm_so]);
	    
	    if (!ht_has_entry(sections, current_section)) {
		current_section_ht = ht_create(NULL);
		ht_add_entry(sections, current_section, current_section_ht);
	    } else {
		ht_get_entry(sections, current_section, &current_section_ht);
	    }
	    continue;
	}

	if (!current_section || *line == '\r' || *line == '\n')
	    continue;
	if (!strcmp(current_section, "Events")) {
	    // parse subsection and event in a similar manner
	} else if (!strcmp(current_section, "HitObjects")) {
	    struct hit_object *ho = calloc(sizeof(*ho), 1);
	    ho_parse(line, ho, version);
	    list_add(hit_objects, ho);
	} else if (!strcmp(current_section, "TimingPoints")) {
	    struct timing_point *tp = malloc(sizeof(*tp));
	    tp_parse(line, tp);
	    list_add(timing_points, tp);
	} else {
	    struct generic_entry ge;
	    default_parser(line, &ge);
	    ht_add_entry(current_section_ht, ge.id, strdup(ge.value));
	}
    }

    free(current_section);
    regfree(&section_delim);
    return sections;
}

