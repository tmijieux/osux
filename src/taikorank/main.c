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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "print.h"

#include "options.h"
#include "config.h"
#include "reading.h"
#include "pattern.h"
#include "accuracy.h"
#include "density.h"
#include "final_star.h"


static int apply_global_options(int argc, const char ** argv)
{
    int i;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != GLOBAL_OPT_PREFIX[0])
	    break;
	i += global_opt_set(argc - i, &argv[i]);
    }
    init_enabled();
    return i;
}

static void tr_initialize(void)
{
    tr_options_initialize();
    tr_config_initialize();

    tr_reading_initialize();
    tr_pattern_initialize();
    tr_accuracy_initialize();
    tr_density_initialize();
    tr_final_star_initialize();
}

int main(int argc, char *argv[])
{
    tr_initialize();

    int nb_map = 0;
    int start = apply_global_options(argc, (const char **) argv);

    #pragma omp parallel
    #pragma omp single
    for (int i = start; i < argc; i++) {
	if (argv[i][0] == LOCAL_OPT_PREFIX[0]) {
	    i += local_opt_set(argc - i, (const char **) &argv[i]);
	} else {
	    nb_map++;
	    struct tr_map * map = trm_new(argv[i]);
	    if (map == NULL)
		continue;

	    map->conf = tr_local_config_copy();
            #pragma omp task firstprivate(map)
	    {
		map->conf->tr_main(map);
		tr_local_config_free(map->conf);
		trm_free(map);
	    }
	}
    }

    if (nb_map == 0) {
	tr_error("No osu file D:");
	print_help();
    }

    return EXIT_SUCCESS;
}
