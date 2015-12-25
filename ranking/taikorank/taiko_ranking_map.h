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

#ifndef TRM_H
#define TRM_H

struct tr_object;

struct tr_map
{
  // Name info
  char * title;
  char * artist;
  char * source;
  char * creator;
  char * diff;

  char * title_uni;
  char * artist_uni;
  unsigned int bms_osu_ID;
  unsigned int diff_osu_ID;
  
  // Song Info
  int mods;
  double od;
  double great_ms;
  double bad_ms;
  
  // Taiko objects
  int nb_object;
  struct tr_object * object; 
  
  // stars *-*
  double density_star;
  double reading_star;
  double pattern_star;
  double accuracy_star;
  double final_star;
};

//----------------------------------------

struct tr_map * trm_new(char * file_name);
struct tr_map * trm_copy(const struct tr_map * map);
void trm_pattern_free(struct tr_map * map);
void trm_free(struct tr_map * map);

int trm_hardest_tro(struct tr_map * map);
int trm_best_influence_tro(struct tr_map * map);
void trm_remove_tro(struct tr_map * map, int x);

void trm_set_mods(struct tr_map * map, int mods);
void trm_compute_stars(struct tr_map * map);

void trm_main(const struct tr_map * map, int mods);

void trm_print_tro(struct tr_map * map, int filter);
void trm_print(struct tr_map * map);
void trm_print_yaml(struct tr_map * map);
void tr_print_yaml_init(void);
void tr_print_yaml_exit(void);
#endif
