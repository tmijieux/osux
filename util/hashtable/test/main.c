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

#include "hash_table.h"

int main(int argc, char *argv[])
{
    struct hash_table *ht = ht_create(0, NULL);
    ht_add_entry(ht, "lulz", "some data", 10);

    int lulz, ret;
    char *buf = ht_get_entry(ht, "lulz", &lulz, &ret);
    printf("%s, %d, %d\n", buf, lulz, ret);

    free(buf);

    ht_free(ht);


    
    /* add_label("TOP_LEVEL", "\\[(.*)\\]"); */
    /* set_root_label("TOP_LEVEL"); */
    
    /* int tl_conf = add_group("TOP_LEVEL", CONFIG_LIKE); */
    /* int tl_proplist = add_group("TOP_LEVEL", PROPERTY_LIST); */
    /* int tl_subsect = add_group("TOP_LEVEL", SUBSECTION); */
    
    /* add_section(tl_conf, "General"); */
    /* add_section(tl_conf, "Editor"); */
    /* add_section(tl_conf, "Metadata"); */
    /* add_section(tl_conf, "Difficulty"); */
    /* add_section(tl_conf, "Colours"); */

    /* add_section(tl_proplist, "TimingPoints"); */
    /* add_section(tl_proplist, "HitObjects"); */
    
    /* add_section(tl_subsect, "Events"); */

        
    /* add_label("SUBSECTION", "//(.*)"); */
    /* int ss_proplist = add_group("SUBSECTION", PROPERTY_LIST); */
    
    /* add_section(ss_proplist, "Background and Video events"); */
    /* add_section(ss_proplist, "Break Periods"); */
    /* add_section(ss_proplist, "Storyboard Layer 0 (Background)"); */
    /* add_section(ss_proplist, "Storyboard Layer 1 (Fail)"); */
    /* add_section(ss_proplist, "Storyboard Layer 2 (Pass)"); */
    /* add_section(ss_proplist, "Storyboard Layer 3 (Foreground)"); */
    /* add_section(ss_proplist, "Storyboard Sound Samples"); */

    /* set_group_parent(ss_proplist, tl_subsect); */
    
    
    return EXIT_SUCCESS;
}
