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
#ifndef CONFIG_H
#define CONFIG_H

extern int OPT_DATABASE;
extern int OPT_PRINT_TRO;
extern int OPT_PRINT_YAML;
extern char * OPT_PRINT_ORDER;

extern char * TR_DB_IP;
extern char * TR_DB_LOGIN;
extern char * TR_DB_PASSWD;

extern int OPT_SCORE;
extern double SCORE_ACC;
extern int (* TRM_METHOD_GET_TRO)(struct tr_map *);

#endif //CONFIG_H
