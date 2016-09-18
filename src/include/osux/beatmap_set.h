#ifndef OSUX_BEATMAP_SET_H
#define OSUX_BEATMAP_SET_H

typedef struct osux_beatmap_set_ osux_beatmap_set;

struct osux_beatmap_set_ {
    uint32_t id;
    int32_t osu_id;

    char *creator;
    char *artist;
    char *artist_unicode;
    char *title;
    char *title_unicode;
    char *display_font;
    char *tags;
    char *source;
    char *directory;
    int32_t status;
};

#endif // OSUX_BEATMAP_SET_H
