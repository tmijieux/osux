#ifndef BEATMAP_SET_H
#define BEATMAP_SET_H

struct osux_beatmap_set {
    uint32_t beatmap_set_id;
    int32_t osu_beatmap_set_id;
    
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

#endif //BEATMAP_SET_H
