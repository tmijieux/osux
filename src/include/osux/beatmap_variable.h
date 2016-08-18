#ifndef OSUX_BEATMAP_VARIABLE_H
#define OSUX_BEATMAP_VARIABLE_H

#ifdef BEATMAP_VARIABLE__IMPL__
#define DEFINE_DEFAULT_VALUE( section, field, type, value, method)      \
    type osux_beatmap_default_value_##field = value;
#endif

#define DECLARE_DEFAULT_VALUE( section, field, type, value, method)     \
    extern type osux_beatmap_default_value_##field;

#include <stdlib.h>
#include <stdbool.h>

static inline double osux_strtod(char *str) { return strtod(str, NULL); }
static inline double osux_strtoull(char *str) { return strtoull(str, NULL, 10); }
static inline bool osux_tobool(char *str) { return !!atoi(str); }


// VALUE(section, fieldname, type, default_value, from_string_conversion_method)

#define DEFAULT_VALUES(VALUE)                                           \
    VALUE( General, AudioFilename, char*, g_strdup(""), g_strdup)       \
    VALUE( General, AudioLeadIn, int64_t, 0, atoi )                         \
    VALUE( General, PreviewTime, int64_t, -1, atoi)                     \
    VALUE( General, Countdown, int64_t, 0, atoi )                           \
    VALUE( General, SampleSet, char*, g_strdup("Normal"), g_strdup)     \
    VALUE( General, StackLeniency, double,  0.7, osux_strtod)           \
    VALUE( General, Mode, int64_t, 0, atoi)                                 \
    VALUE( General, LetterboxInBreaks, bool, true,  osux_tobool)        \
    VALUE( General, WidescreenStoryboard, bool, true, osux_tobool)      \
                                                                        \
    VALUE( Editor, DistanceSpacing, double, 1, osux_strtod)             \
    VALUE( Editor, BeatDivisor, int64_t, 1, osux_strtod)                    \
    VALUE( Editor, GridSize, int64_t, 1, atoi )                             \
    VALUE( Editor, TimelineZoom, double, 1, osux_strtod)                \
                                                                        \
    VALUE( Metadata, Title,char*, g_strdup(""), g_strdup)               \
    VALUE( Metadata, TitleUnicode, char*, g_strdup(""), g_strdup)       \
    VALUE( Metadata, Artist, char*, g_strdup(""), g_strdup )            \
    VALUE( Metadata, ArtistUnicode, char*, g_strdup(""), g_strdup)      \
    VALUE( Metadata, Creator, char* ,g_strdup(""), g_strdup )           \
    VALUE( Metadata, Version, char*, g_strdup(""), g_strdup )           \
    VALUE( Metadata, Source, char*, g_strdup(""), g_strdup)             \
    VALUE( Metadata, BeatmapID, int64_t, 0, atoi)                           \
    VALUE( Metadata, BeatmapSetID, int64_t, -1, atoi)                       \
                                                                        \
    VALUE( Difficulty, HPDrainRate, double, 5., osux_strtod)            \
    VALUE( Difficulty, CircleSize, double, 5., osux_strtod)             \
    VALUE( Difficulty, OverallDifficulty, double, 5., osux_strtod )     \
    VALUE( Difficulty, ApproachRate, double, 5., osux_strtod)           \
    VALUE( Difficulty, SliderMultiplier, double, 1., osux_strtod)       \
    VALUE( Difficulty, SliderTickRate, double, 1., osux_strtod)         \

/* DEFAULT_VALUES(DECLARE_DEFAULT_VALUE); */

#endif // OSUX_BEATMAP_VARIABLE_H
