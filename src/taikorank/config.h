/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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
#ifndef CONFIG_H
#define CONFIG_H

#include "osux/beatmap_database.h"

enum score_input {
    SCORE_INPUT_ACC = 0,
    SCORE_INPUT_GGM = 1
};

enum score_method {
    SCORE_INPUT_HARDEST   = 0,
    SCORE_INPUT_INFLUENCE = 1
};

enum tr_main {
    MAIN_MAP   = 0,
    MAIN_SCORE = 1
};

struct tr_global_config {
    int autoconvert_enable;

    int db_enable;
    char * db_ip;
    char * db_login;
    char * db_passwd;

    int print_tro;
    int print_yaml;
    int print_filter;
    char * print_order;

    int beatmap_db_enable;
    char * beatmap_db_path;
    osux_beatmap_db beatmap_db;
};

struct tr_local_config {
    int mods;
    int flat;
    int no_bonus;

    int quick;
    void (* tr_main)(const struct tr_map *);
    int (* trm_method_get_tro)(struct tr_map *);

    enum score_input input;
    double acc;
    int good;
    int miss;

    double step;
};

extern struct tr_local_config * LOCAL_CONFIG;
extern struct tr_global_config * GLOBAL_CONFIG;

void tr_config_initialize(void);
void tr_global_config_print(const struct tr_global_config *conf);

void tr_global_config_free(struct tr_global_config *conf);
void tr_local_config_free(struct tr_local_config *conf);
struct tr_local_config * tr_local_config_copy(void);

void global_config_set_filter(const char * filter);
void local_config_set_mods(const char * mods);
void local_config_set_tr_main(enum tr_main i);

void init_enabled(void);
void ht_conf_db_init(void);

#endif //CONFIG_H
