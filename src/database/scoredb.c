#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "replay/replay.h"

struct score {
    struct replay score_replay;
    // normally dont hold any replay data; only the score part
    
    uint64_t OnlineScoreID;
};

struct score_beatmap {
    char *beatmap_hash;
    uint32_t scores_number;
    struct score scores;
};

struct scoredb {
    uint32_t osu_version;
    uint32_t beatmaps_number;
    struct score_beatmap *beatmaps;
};
