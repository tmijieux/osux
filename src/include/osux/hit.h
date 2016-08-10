#ifndef OSUX_HIT_H
#define OSUX_HIT_H

#include <string.h>

typedef struct osux_hit_ osux_hit;

#include "osux/util.h"
#include "osux/mods.h"
#include "osux/hitobject.h"
#include "osux/replay.h"

inline double osux_base_approach_rate_length(double approach_rate)
{
    if (approach_rate >= 5)
        return 1200. - 150 * approach_rate;
    return 1800 - 120 * approach_rate;
}

extern char const *hit_str[];

typedef enum osux_hit_type_ {
    HIT_RAINBOW_300 = 0,
    HIT_300,
    HIT_200,
    HIT_100,
    HIT_50,
    HIT_MISS,
    MAX_HIT_TYPE,
} osux_hit_type;

struct osux_hit_ {
    osux_hit_type hit_type;
    bool hitted; // can be true even if hit_type == hit_miss
};

typedef struct osux_keypress_ {
    int64_t offset;
    uint32_t key;
    bool release;
} osux_keypress;

typedef struct osux_hits_ {
    int game_mode;
    int mods;
    int overall_difficulty;
    
    osux_replay_data *data;
    size_t data_count;
    
    osux_keypress *keypress;
    size_t keypress_count;

    /* public */
    osux_hit *hits;
    size_t hits_size;
} osux_hits;


int osux_hits_init(osux_hits *hits,
                   osux_beatmap const *beatmap, osux_replay const *replay);
int osux_hits_free(osux_hits *hits);

#endif // OSUX_HIT_H
