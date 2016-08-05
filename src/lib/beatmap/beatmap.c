#include <glib.h>
#include <float.h>

#include "osux/md5.h"
#include "osux/error.h"

#include "osux/beatmap.h"
#include "osux/beatmap_variable.h"
#include "osux/hitobject.h"
#include "osux/timingpoint.h"
#include "osux/event.h"
#include "osux/util.h"

int osux_beatmap_free(osux_beatmap *beatmap)
{
    if (beatmap == NULL)
        return -1;

    g_free(beatmap->file_path);
    g_free(beatmap->osu_filename);
    g_free(beatmap->md5_hash);

    g_free(beatmap->AudioFilename);
    g_free(beatmap->SampleSet);
    g_free(beatmap->Bookmarks);
    g_free(beatmap->Title);
    g_free(beatmap->TitleUnicode);
    g_free(beatmap->Artist);
    g_free(beatmap->ArtistUnicode);
    g_free(beatmap->Creator);
    g_free(beatmap->Version);
    g_free(beatmap->Source);

    g_strfreev(beatmap->tags);
    g_free(beatmap->colors);

    for (unsigned i = 0; i < beatmap->timingpoint_count; ++i)
	osux_timingpoint_free(&beatmap->timingpoints[i]);
    for (unsigned i = 0; i < beatmap->hitobject_count; ++i)
	osux_hitobject_free(&beatmap->hitobjects[i]);
    for (unsigned i = 0; i < beatmap->event_count; ++i)
	osux_event_free(&beatmap->events[i]);

    g_free(beatmap->events);
    g_free(beatmap->hitobjects);
    g_free(beatmap->timingpoints);

    if (beatmap->sections != NULL)
        osux_hashtable_delete(beatmap->sections);

    return 0;
}

static int parse_osu_version(osux_beatmap *beatmap, FILE *file)
{
    static uint8_t BOM[] = { 0xEF, 0xBB, 0xBF };
    static GRegex *regexp = NULL;

    int err = 0;
    char *line = osux_getline(file);

    if (line == NULL)
        return OSUX_ERR_BAD_OSU_VERSION;

    if (memcmp(line, BOM, sizeof BOM) == 0) {
        beatmap->byte_order_mark = true;
        line += sizeof BOM;
    }

    if (regexp == NULL)
        regexp = g_regex_new("^osu file format v([0-9]+)$", 0, 0, NULL);

    GMatchInfo *info = NULL;
    if (!g_regex_match(regexp, line, 0, &info)) {
        g_free(line);
        g_match_info_free(info);
        return OSUX_ERR_BAD_OSU_VERSION;
    }

    if (g_match_info_matches(info)) {
        gchar *version = g_match_info_fetch(info, 1);
        beatmap->osu_version = atoi(version);
        g_free(version);
    } else
        err = OSUX_ERR_BAD_OSU_VERSION;

    g_free(line);
    g_match_info_free(info);
    return err;
}

static int compute_metadata(osux_beatmap *beatmap, FILE *file)
{
    int err;
    beatmap->md5_hash = osux_get_file_hashstr(beatmap->file_path);

    /* GFile *gfile; */
    /* GFileInfo *ginfo; */
    /* gfile = g_file_new_for_path(file_path); */
    /* ginfo = g_file_query_info(gfile, G_FILE_ATTRIBUTE_TIME_MODIFIED, */
    /*                          G_FILE_QUERY_INFO_NONE, NULL, NULL); */
    /* g_file_info_get_last_modification(ginfo, &beatmap->last_modification); */
    /* g_object_unref(ginfo); */
    /* g_object_unref(gfile); */

    if ((err = parse_osu_version(beatmap, file)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }

    return 0;
}

static bool line_is_empty_or_comment(char *line)
{
    return strcmp("", line) == 0 ||
        (strlen(line) >= 2 && strncmp("//", line, 2) == 0);
}

static bool get_new_section(char const *line, char **section_name)
{
    static GRegex *regexp = NULL;
    if (regexp == NULL)
        regexp = g_regex_new("^\\[(.*)\\]$", 0, 0, NULL);

    GMatchInfo *info = NULL;
    if (!g_regex_match(regexp, line, 0, &info)) {
        g_match_info_free(info);
        return false;
    }

    if (g_match_info_matches(info)) {
        g_free(*section_name);
        *section_name = g_match_info_fetch(info, 1);
        g_match_info_free(info);
        return true;
    }
    g_match_info_free(info);
    return false;
}

static int parse_option_entry(
    char const *line, osux_hashtable *section)
{
    char *sep = strstr(line, ":");
    if (sep == NULL)
        return OSUX_ERR_MALFORMED_OSU_FILE;
    char **split = g_strsplit(line, ":", 2);
    osux_hashtable_insert(section, split[0], g_strdup(split[1]));
    g_strfreev(split);
    return 0;
}

#define CHECK_TIMING_POINT(x)
#define CHECK_HIT_OBJECT(r, x)                                          \
    if (r < 0) {                                                        \
        osux_debug("failed to parse hitobject line %d\n", line_count);  \
    }


#define CHECK_EVENT(x)


#define UPDATE_STAT_HO_COUNT(beatmap, hitobject)        \
    do {                                                \
        if (HIT_OBJECT_IS_SLIDER(hitobject)) {          \
            beatmap->sliders ++;                        \
        } else if (HIT_OBJECT_IS_CIRCLE(hitobject)) {   \
            beatmap->circles ++;                        \
        } else if (HIT_OBJECT_IS_SPINNER(hitobject)) {  \
            beatmap->spinners ++;                       \
        }                                               \
    } while(0)

#define UPDATE_STAT_BPM(beatmap, timingpoint)                   \
    do {                                                        \
        if ((timingpoint)->inherited) break;                    \
        double bpm = TP_GET_BPM(timingpoint);                   \
        if (bpm > (beatmap)->bpm_max) {                         \
            (beatmap)->bpm_max = bpm;                           \
        }                                                       \
        if (bpm < (beatmap)->bpm_min) {                         \
            (beatmap)->bpm_min = bpm;                           \
        }                                                       \
        beatmap->bpm_avg *= (beatmap)->hitobject_count;         \
        beatmap->bpm_avg += bpm;                                \
        beatmap->bpm_avg /= (beatmap)->hitobject_count+1;       \
    } while(0)

#define ALLOC_ARRAY(array_var, size_var, size_literal)                  \
    do {                                                                \
        array_var = g_malloc0((size_literal) * sizeof(*(array_var)));   \
        size_var = (size_literal);                                      \
    } while (0)

static int parse_objects(osux_beatmap *beatmap, FILE *file)
{
    osux_timingpoint const *last_non_inherited = NULL;
    osux_hashtable *current_section = NULL;
    char *section_name = NULL;

    beatmap->sections = osux_hashtable_new_full(
        0, (void(*)(void*)) &osux_hashtable_delete);

    ALLOC_ARRAY(beatmap->hitobjects, beatmap->hitobject_bufsize, 500);
    ALLOC_ARRAY(beatmap->timingpoints, beatmap->timingpoint_bufsize, 500);

    beatmap->bpm_min = DBL_MAX;
    ALLOC_ARRAY(beatmap->events, beatmap->event_bufsize, 500);


    char *line;
    int line_count = 0;
    for (line = NULL; (line = osux_getline(file)) != NULL; g_free(line)) {
        ++ line_count;
        if (line_is_empty_or_comment(line))
            continue;

        if (get_new_section(line, &section_name)) {
            current_section = osux_hashtable_new_full(0, g_free);
            osux_hashtable_insert(
                beatmap->sections, section_name, current_section);
            //printf("section='%s'\n", section_name);
            continue;
        }

        if (!strcmp(section_name, "TimingPoints")) {
            HANDLE_ARRAY_SIZE(beatmap->timingpoints,
                              beatmap->timingpoint_count,
                              beatmap->timingpoint_bufsize);
            osux_timingpoint_init(
                &beatmap->timingpoints[beatmap->timingpoint_count],
                &last_non_inherited, line, beatmap->osu_version);
            CHECK_TIMING_POINT(&beatmap->timingpoints[beatmap->timingpoint_count]);
            UPDATE_STAT_BPM(
                beatmap, &beatmap->timingpoints[beatmap->timingpoint_count]);
            ++ beatmap->timingpoint_count;
            continue;
        }

        if (!strcmp(section_name, "HitObjects")) {
            HANDLE_ARRAY_SIZE(beatmap->hitobjects,
                              beatmap->hitobject_count,
                              beatmap->hitobject_bufsize);
            int r = osux_hitobject_init(&beatmap->hitobjects[beatmap->hitobject_count],
                                line, beatmap->osu_version);
            CHECK_HIT_OBJECT(r, &beatmap->hitobjects[beatmap->hitobject_count]);
            UPDATE_STAT_HO_COUNT(
                beatmap, &beatmap->hitobjects[beatmap->hitobject_count]);
            ++ beatmap->hitobject_count;
            continue;
        }

        if (!strcmp(section_name, "Events")) {
            HANDLE_ARRAY_SIZE(beatmap->events,
                              beatmap->event_count,
                              beatmap->event_bufsize);
            osux_event_init(&beatmap->events[beatmap->event_count],
                       line, beatmap->osu_version);
            CHECK_EVENT(&beatmap->events[beatmap->event_count]);
            ++ beatmap->event_count;
            continue;
        }

        parse_option_entry(line, current_section);
    }
    g_free(section_name);
    return 0;
}

#define FETCH( section, field, type, default_value, method )            \
    do {                                                                \
        osux_hashtable *section_ = NULL;                                \
        osux_hashtable_lookup(beatmap->sections, #section, &section_);  \
                                                                        \
        char *str_ = NULL;                                              \
        if (osux_hashtable_lookup(section_, #field, &str_) < 0) {       \
            beatmap->field = (default_value);                           \
        } else {                                                        \
            beatmap->field = method(str_);                              \
        }                                                               \
    } while (0);


static int fetch_variables(osux_beatmap *beatmap)
{
    DEFAULT_VALUES(FETCH);
    return 0;
}

int osux_beatmap_init(osux_beatmap *beatmap, char const *file_path)
{
    int err = 0;
    memset(beatmap, 0, sizeof *beatmap);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        osux_error("Cannot open file for reading: '%s'\n", file_path);
        return OSUX_ERR_FILE_PERM;
    }

    beatmap->file_path = strdup(file_path);
    beatmap->osu_filename = g_path_get_basename(file_path);

    if ((err = compute_metadata(beatmap, file)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }

    if ((err = parse_objects(beatmap, file)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }
    fclose(file);

    if ((err = fetch_variables(beatmap)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }

    return 0;
}
