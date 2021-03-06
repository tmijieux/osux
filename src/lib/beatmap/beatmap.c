#include <glib.h>
#include <glib/gstdio.h>
#include <float.h>

#include "osux/md5.h"
#include "osux/error.h"

#include "osux/beatmap.h"
#include "osux/beatmap_variable.h"
#include "osux/hitobject.h"
#include "osux/timingpoint.h"
#include "osux/event.h"
#include "osux/util.h"
#include "osux/hitsound.h"
#include "osux/mods.h"

int osux_beatmap_free(osux_beatmap *beatmap)
{
    if (beatmap == NULL)
        return -OSUX_ERR_INVAL;

    g_free(beatmap->file_path);
    g_free(beatmap->osu_filename);
    g_free(beatmap->md5_hash);

    g_free(beatmap->AudioFilename);
    g_free(beatmap->bookmarks);
    g_free(beatmap->Title);
    g_free(beatmap->TitleUnicode);
    g_free(beatmap->Artist);
    g_free(beatmap->ArtistUnicode);
    g_free(beatmap->Creator);
    g_free(beatmap->Version);
    g_free(beatmap->Source);
    g_free(beatmap->Tags);

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

    //this allow multiple free to be harmless:
    memset(beatmap, 0, sizeof *beatmap);
    return 0;
}

static inline bool line_is_empty_or_comment(char *line)
{
    return strcmp("", line) == 0 ||
        (strlen(line) >= 2 && strncmp("//", line, 2) == 0);
}

static int parse_osu_version(osux_beatmap *beatmap, GIOChannel *file)
{
    int err = 0;
    static GRegex *regexp = NULL;
    char *line =  NULL;
    do {
        g_free(line);
        err = osux_getline(file, &line);
    } while (!err && line_is_empty_or_comment(line));

    if (err)
        return err;

    if (regexp == NULL)
        regexp = g_regex_new("format v([0-9]+)$", 0, 0, NULL);

    GMatchInfo *info = NULL;
    if (!g_regex_match(regexp, line, 0, &info)) {
        err = -OSUX_ERR_BAD_OSU_VERSION;
        goto finally;
    }

    if (g_match_info_matches(info)) {
        gchar *version = g_match_info_fetch(info, 1);
        beatmap->osu_version = atoi(version);
        g_free(version);
    } else {
        printf("debug2\n");
        err = -OSUX_ERR_BAD_OSU_VERSION;
    }

finally:
    g_free(line);
    g_match_info_free(info);
    return err;
}

static int compute_metadata(osux_beatmap *beatmap, GIOChannel *file)
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

    if ((err = parse_osu_version(beatmap, file)) < 0)
        return err;
    return 0;
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
        return -OSUX_ERR_MALFORMED_OSU_FILE;
    char **split = g_strsplit(line, ":", 2);
    osux_hashtable_insert( section,
                           g_strstrip(split[0]),
                           g_strchug(g_strdup(split[1])) );
    g_strfreev(split);
    return 0;
}

#define CHECK_OBJECT(r, s)                              \
    if ((r) < 0) {                                      \
        osux_error("%s line %d\n", s, line_count);      \
        return r;                                       \
    }

#define CHECK_COLOR(r, x)                                               \
    if ((r) < 0) {                                                      \
        osux_warning("invalid color line %d ignored: %s %s\n",          \
                     line_count, osux_errmsg((r)), beatmap->osu_filename); \
        --beatmap->color_count;                                         \
    }

#define CHECK_TIMING_POINT(r, x) CHECK_OBJECT(r, "timing point")
#define CHECK_HIT_OBJECT(r, x) CHECK_OBJECT(r, "hit object")
#define CHECK_EVENT(r, x) CHECK_OBJECT(r, "event")

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
        if (bpm > (beatmap)->bpm_max)                           \
            (beatmap)->bpm_max = bpm;                           \
        if (bpm < (beatmap)->bpm_min)                           \
            (beatmap)->bpm_min = bpm;                           \
        beatmap->bpm_avg *= (beatmap)->hitobject_count;         \
        beatmap->bpm_avg += bpm;                                \
        beatmap->bpm_avg /= (beatmap)->hitobject_count+1;       \
    } while(0)

static gint sort_color(osux_color *a, osux_color *b)
{
    return a->id - b->id;
}

#define UPDATE_COMBO_COLOURS(beatmap, color)                    \
    do {                                                        \
        if ((color)->type == COLOR_COMBO) {                     \
            GList *l = (beatmap)->combo_colours;                \
            l = g_list_insert_sorted(l, (color),                \
                                     (GCompareFunc)sort_color); \
            (beatmap)->combo_colours = l;                       \
        }                                                       \
    } while(0)

#define COLOR_INIT(elem, line, version, beatmap)                \
    do {                                                        \
        int r = osux_color_init(&(elem), (line), (version));    \
        CHECK_COLOR(r, &(elem));                                \
        UPDATE_COMBO_COLOURS((beatmap), &(elem));               \
    } while(0)

#define EVENT_INIT(elem, line, version)                         \
    do {                                                        \
        int r = osux_event_init(&(elem), (line), (version));    \
        CHECK_EVENT(r, &(elem));                                \
    } while(0)

#define HITOBJECT_INIT(elem, line, version, beatmap)                    \
    do {                                                                \
        int r = osux_hitobject_init(&(elem), (line), (version));        \
        CHECK_HIT_OBJECT(r, &(elem));                                   \
        UPDATE_STAT_HO_COUNT((beatmap), &(elem));                       \
    } while(0)

#define TIMINGPOINT_INIT(elem, line, version, beatmap)                  \
    do {                                                                \
        int r = osux_timingpoint_init(&(elem), (line), (version));      \
        CHECK_TIMING_POINT(r, &(elem));                                 \
        UPDATE_STAT_BPM(beatmap, &(elem));                              \
    } while(0)

static int parse_objects(osux_beatmap *beatmap, GIOChannel *file)
{
    int err = 0;
    osux_hashtable *current_section = NULL;
    char *section_name = NULL;

    beatmap->bpm_min = DBL_MAX;
    beatmap->sections = osux_hashtable_new_full(
        0, (void(*)(void*)) &osux_hashtable_delete);

    ALLOC_ARRAY(beatmap->hitobjects, beatmap->hitobject_bufsize, 500);
    ALLOC_ARRAY(beatmap->timingpoints, beatmap->timingpoint_bufsize, 500);
    ALLOC_ARRAY(beatmap->events, beatmap->event_bufsize, 500);
    ALLOC_ARRAY(beatmap->colors, beatmap->color_bufsize, 20);

    char *line = NULL;
    int line_count = 0;
    for (line = NULL; (err = osux_getline(file, &line))==0; g_free(line)) {
        ++ line_count;
        if (line_is_empty_or_comment(line))
            continue;

        if (get_new_section(line, &section_name)) {
            current_section = osux_hashtable_new_full(0, g_free);
            osux_hashtable_insert(
                beatmap->sections, section_name, current_section);
            //printf("section='%s'\n", section_name);
        } else if (!strcmp(section_name, "TimingPoints"))
            ARRAY_APPEND(beatmap->timingpoint, TIMINGPOINT_INIT,
                         line, beatmap->osu_version, beatmap);
        else if (!strcmp(section_name, "HitObjects"))
            ARRAY_APPEND(beatmap->hitobject, HITOBJECT_INIT,
                         line, beatmap->osu_version, beatmap);
        else if (!strcmp(section_name, "Events"))
            ARRAY_APPEND(beatmap->event, EVENT_INIT, line, beatmap->osu_version);
        else if (!strcmp(section_name, "Colours"))
            ARRAY_APPEND(beatmap->color, COLOR_INIT,
                         line, beatmap->osu_version, beatmap);
        else
            parse_option_entry(line, current_section);
    }
    g_free(section_name);
    if (err == 1) err = 0;
    return err;
}

#define FETCH( section, field, type, default_value, method )            \
    do {                                                                \
        osux_hashtable *section_ = NULL;                                \
        if (osux_hashtable_lookup(beatmap->sections, #section, &section_) < 0) { \
            /*osux_debug("No section '%s'\n", #section);*/              \
            beatmap->field = (default_value);                           \
            break;                                                      \
        }                                                               \
                                                                        \
        char *str_ = NULL;                                              \
        if (osux_hashtable_lookup(section_, #field, &str_) < 0) {       \
            beatmap->field = (default_value);                           \
        } else {                                                        \
            beatmap->field = method(str_);                              \
        }                                                               \
    } while (0);


static void fetch_bookmarks(osux_beatmap *beatmap)
{
    osux_hashtable *section = NULL;
    char *bookmarks = NULL;
    osux_hashtable_lookup(beatmap->sections, "Editor", &section);
    if (section != NULL) {
        osux_hashtable_lookup(section, "Bookmarks", &bookmarks);
    } else {
        osux_hashtable_lookup(beatmap->sections, "General", &section);
        if (section != NULL)
            osux_hashtable_lookup(section, "EditorBookmarks", &bookmarks);
    }

    if (bookmarks == NULL)
        return;

    char **split = g_strsplit(bookmarks, ",", 0);
    unsigned size = strsplit_size(split);
    ALLOC_ARRAY(beatmap->bookmarks, beatmap->bookmark_bufsize, size);
    beatmap->bookmark_count = size;
    for (unsigned i = 0; i < size; ++i)
        beatmap->bookmarks[i] = atoi(split[i]);
    g_strfreev(split);
}

static void fetch_tags(osux_beatmap *beatmap)
{
    osux_hashtable *section = NULL;
    char *tags = NULL;
    osux_hashtable_lookup(beatmap->sections, "Metadata", &section);
    if (section != NULL) {
        osux_hashtable_lookup(section, "Tags", &tags);
        beatmap->tags_orig = g_strdup("");
        if (tags != NULL) {
            g_free(beatmap->tags_orig);
            beatmap->tags_orig = g_strdup(tags);
            beatmap->tags = g_strsplit(tags, " ", 0);
            beatmap->tag_count = strsplit_size(beatmap->tags);
        }
    }
}

#define MATCH_SAMPLE_SET_(value_, caps_, pretty_)       \
    do {                                                \
        if (!g_strcmp0((pretty_), sample_set)) {        \
            return (value_);                            \
        }                                               \
    } while (0);


static int64_t parse_sample_set(gchar const *sample_set)
{
    SAMPLE_SETS(MATCH_SAMPLE_SET_);
    return -1;
}

static int fetch_variables(osux_beatmap *beatmap)
{
    DEFAULT_VALUES(FETCH);
    fetch_bookmarks(beatmap);
    fetch_tags(beatmap);
    return 0;
}

static void
prepare_colors(osux_beatmap *beatmap)
{
    if (!osux_color_array_contains_type(
        beatmap->colors, beatmap->color_count, COLOR_COMBO))
    {
        osux_color c;
        osux_color_init(&c, "Combo1 : 255,192,0", 15);
        osux_beatmap_append_color(beatmap, &c);

        osux_color_init(&c, "Combo2 : 0,202,0", 15);
        osux_beatmap_append_color(beatmap, &c);

        osux_color_init(&c, "Combo3 : 18,124,255", 15);
        osux_beatmap_append_color(beatmap, &c);

        osux_color_init(&c, "Combo4 : 242,24,57", 15);
        osux_beatmap_append_color(beatmap, &c);
    }
}

static void
osux_combo_init(osux_combo *combo, osux_beatmap *beatmap)
{
    memset(combo, 0, sizeof *combo);
    combo->colours = beatmap->combo_colours;
    combo->current = g_list_last(combo->colours);
}

static void osux_combo_next(osux_combo *combo, osux_hitobject *ho)
{
    if (HIT_OBJECT_IS_NEWCOMBO(ho)) {
        if (combo->current->next == NULL)
            combo->current = combo->colours;
        else
            combo->current = combo->current->next;
        ++ combo->id;
        combo->pos = 1;
    } else
        ++ combo->pos;
}

static osux_color*
osux_combo_colour(osux_combo *combo)
{
    return (osux_color*) combo->current->data;
}

static int prepare_hitobjects(osux_beatmap *beatmap)
{
    int err = 0;
    uint32_t current_tp = 0;
    osux_combo combo;
    osux_combo_init(&combo, beatmap);

    for (uint32_t i = 0; !err && i < beatmap->hitobject_count; ++i) {
	osux_hitobject *ho = &beatmap->hitobjects[i];
        // for each hit object
        // compute timing point applying on this hit object
	while (current_tp < (beatmap->timingpoint_count-1) &&
	       beatmap->timingpoints[current_tp + 1].offset <= ho->offset)
	    current_tp++;

        osux_combo_next(&combo, ho);
	err = osux_hitobject_prepare(ho, combo.id, combo.pos,
                                     osux_combo_colour(&combo),
                                     &beatmap->timingpoints[current_tp]);
    }
    return err;
}

int osux_beatmap_prepare(osux_beatmap *beatmap)
{
    int err = 0;
    osux_timingpoint const *last_non_inherited = NULL;

    for (uint32_t i = 0; !err && i < beatmap->timingpoint_count; ++i) {
        err = osux_timingpoint_prepare(&beatmap->timingpoints[i],
                                       &last_non_inherited,
                                       beatmap->SliderMultiplier);
    }
    prepare_colors(beatmap);
    err = prepare_hitobjects(beatmap);

    // build the event tree
    for (uint32_t i = 0; !err && i < beatmap->event_count; ++i) {
	osux_event *ev = &beatmap->events[i];
        err = osux_event_build_tree(ev);
    }
    for (uint32_t i = 0; !err && i < beatmap->event_count; ++i) {
	osux_event *ev = &beatmap->events[i];
        err = osux_event_prepare(ev);
    }
    return err;
}

int osux_beatmap_init(osux_beatmap *beatmap, char const *file_path)
{
    int err = 0;
    memset(beatmap, 0, sizeof *beatmap);

    GIOChannel *file = osux_open_text_file_reading(file_path);
    if (file == NULL)
        return -OSUX_ERR_FILE_ACCESS;

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
    g_io_channel_unref(file);
    file = NULL;
    if ((err = fetch_variables(beatmap)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }
    if ((err = osux_beatmap_prepare(beatmap)) < 0) {
        osux_beatmap_free(beatmap);
        return err;
    }
    return 0;
}

void osux_beatmap_append_hitobject(osux_beatmap *beatmap, osux_hitobject *ho)
{
    HANDLE_ARRAY_SIZE(beatmap->hitobjects,
                      beatmap->hitobject_count,
                      beatmap->hitobject_bufsize);
    osux_hitobject_move(ho, &beatmap->hitobjects[beatmap->hitobject_count]);
    UPDATE_STAT_HO_COUNT(
        beatmap, &beatmap->hitobjects[beatmap->hitobject_count]);
    ++ beatmap->hitobject_count;
}

void osux_beatmap_append_timingpoint(osux_beatmap *beatmap, osux_timingpoint *tp)
{
    HANDLE_ARRAY_SIZE(beatmap->timingpoints,
                      beatmap->timingpoint_count,
                      beatmap->timingpoint_bufsize);
    osux_timingpoint_move(tp, &beatmap->timingpoints[beatmap->timingpoint_count]);
    UPDATE_STAT_BPM(
        beatmap, &beatmap->timingpoints[beatmap->timingpoint_count]);
    ++ beatmap->timingpoint_count;
}

void osux_beatmap_append_event(osux_beatmap *beatmap, osux_event *ev)
{
    HANDLE_ARRAY_SIZE(beatmap->events,
                      beatmap->event_count,
                      beatmap->event_bufsize);
    osux_event_move(ev, &beatmap->events[beatmap->event_count]);
    ++ beatmap->event_count;
}

void osux_beatmap_append_color(osux_beatmap *beatmap, osux_color *c)
{
    HANDLE_ARRAY_SIZE(beatmap->colors,
                      beatmap->color_count,
                      beatmap->color_bufsize);
    osux_color_move(c, &beatmap->colors[beatmap->color_count]);
    c = &beatmap->colors[beatmap->color_count];
    UPDATE_COMBO_COLOURS(beatmap, c);
    ++ beatmap->color_count;
}

/* return the circle radius in scene pixel */
int osux_get_circle_size(double circle_size, int mods)
{
    if (mods & MOD_HARDROCK)
        circle_size = MIN(7.0, 1.4 * circle_size);
    return 57 - 4 * circle_size;
}
