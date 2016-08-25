#include "osux/hitsound.h"

#define TO_STRING_ARRAY_(value_, caps_, pretty_)        \
    [(value_)] = (pretty_),

static gchar const *sample_set_str[] = {
    SAMPLE_SETS(TO_STRING_ARRAY_)
};

gchar const *osux_sample_set_get_name(int sample_set)
{
    if (sample_set < 0 || sample_set >= MAX_SAMPLE_TYPE)
        return NULL;
    return sample_set_str[sample_set];
}

gchar const *osux_sample_set_get_localized_name(int sample_set)
{
    if (sample_set < 0 || sample_set >= MAX_SAMPLE_TYPE)
        return NULL;
    return gettext(sample_set_str[sample_set]);
}
