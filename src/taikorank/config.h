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

enum score_input {
    SCORE_INPUT_ACC = 0,
    SCORE_INPUT_GGM = 1
};

enum score_method {
    SCORE_INPUT_HARDEST   = 0,
    SCORE_INPUT_INFLUENCE = 1
};

struct tr_config {
    int mods;
    int quick;
    void (* tr_main)(const struct tr_map *);
    int (* trm_method_get_tro)(struct tr_map *);

    enum score_input input;
    double acc;
    int good;
    int miss;
};

extern int OPT_AUTOCONVERT;

extern int OPT_DATABASE;
extern int OPT_PRINT_TRO;
extern int OPT_PRINT_YAML;
extern int OPT_PRINT_FILTER;
extern char * OPT_PRINT_ORDER;

extern int OPT_FLAT;
extern int OPT_NO_BONUS;

extern char * TR_DB_IP;
extern char * TR_DB_LOGIN;
extern char * TR_DB_PASSWD;

extern struct osux_db * ODB;
extern char * OPT_ODB_PATH;
extern char * OPT_ODB_SGDIR;
extern char * OPT_ODB_STATE;

extern struct tr_config * CONF;

void tr_config_free(struct tr_config * conf);
struct tr_config * tr_config_copy(struct tr_config * conf);

void config_odb_apply_state(char odb_state);
void config_set_mods(const char * mods);
void config_set_filter(char * filter);
void config_set_tr_main(int score);
void config_score(void);

void ht_conf_db_init(void);

#endif //CONFIG_H
