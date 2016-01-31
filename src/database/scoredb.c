#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "replay/replay.h"

struct osux_score {
    struct osux_replay score_replay;
    // normally dont hold any replay data; only the score part
    
    uint64_t OnlineScoreID;
};


