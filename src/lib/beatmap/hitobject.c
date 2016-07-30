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

#include "osux/hitobject.h"

void ho_print(struct hit_object *ho, int version, FILE *f)
{
    fprintf(f, "%d,%d,%d,%d,%d",
	    ho->x, ho->y, ho->offset, ho->type, ho->hs.sample);
    
    switch ( TYPE_OF(ho) ) {
    case HO_SLIDER:
	fprintf(f, ",%c", ho->sli.type);
	for (unsigned i = 0; i < ho->sli.point_count; ++i)
	    fprintf(f, "|%d:%d", ho->sli.pos[i].x, ho->sli.pos[i].y);
	fprintf(f, ",%d,%.15g", ho->sli.repeat, ho->sli.length);
	if (ho->sli.hs.additional) {
	    fprintf(f, ",");
	    for (unsigned i = 0; i < ho->sli.repeat+1; ++i) {
		if (i >= 1) fprintf(f, "|");
		fprintf(f, "%d", ho->sli.hs.dat[i].sample);
	    }
	    fprintf(f, ",");
	    for (unsigned i = 0; i < ho->sli.repeat+1; ++i) {
		if (i >= 1) fprintf(f, "|");
		fprintf(f, "%d:%d", ho->sli.hs.dat[i].st,
		       ho->sli.hs.dat[i].st_additional);
	    }
	}
	break;
    case HO_SPINNER:
	fprintf(f, ",%d", ho->spi.end_offset);
	break;
    }
    if (ho->hs.additional) {
	fprintf(f, ",%d:%d:%d",
	       ho->hs.st,
	       ho->hs.st_additional,
	       ho->hs.sample_set_index);
	if (version > 11) {
	    fprintf(f, ":%d:%s",
		   ho->hs.volume,
		   ho->hs.sfx_filename);
	}
    }
    fprintf(f, "\r\n");
}

void ho_free(struct hit_object *ho)
{
    if ( TYPE_OF(ho) == HO_SLIDER ) {
	free(ho->sli.pos);
	if (ho->sli.hs.additional)
	    free(ho->sli.hs.dat);
    }
    if (ho->hs.additional)
	free(ho->hs.sfx_filename);
}

