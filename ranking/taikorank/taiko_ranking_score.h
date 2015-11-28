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

#ifndef TAIKO_RANKING_SCORE_H
#define TAIKO_RANKING_SCORE_H

struct tr_map;

struct tr_score
{
  const struct tr_map * origin;
  double acc;
  
  struct tr_map * map;
  double current_acc;
};

void trs_main(const struct tr_map * map, int mods, double acc);
struct tr_score * trs_new(const struct tr_map * map, int mods,
			  double acc);
void trs_free(struct tr_score * score);

void trs_compute(struct tr_score * score);

void trs_print(struct tr_score * score);

#endif //TAIKO_RANKING_SCORE_H
