#ifndef KEYS_H
#define KEYS_H

// TAIKO
enum replay_taiko_keys {
    LEFT_DON    =   1,
    LEFT_KAT    =   2,
    RIGHT_DON   =   4,
    RIGHT_KAT   =   8
};


enum replay_std_keys {
    MOUSE_LEFT   =  1,
    MOUSE_RIGHT  =  2,
    KEY_LEFT     =  4 + 1,
    KEY_RIGHT    =  8 + 2,
    SMOKE        =  16
};


enum replay_ctb_keys {
    CTB_DASH = 1
};

enum replay_mania_keys {
    // This is encoded in the x field; NOT IN the key field
    // keys left to right
    
    MANIA_1   =  1,
    MANIA_2   =  2,
    MANIA_3   =  4,
    MANIA_4   =  8,
    MANIA_5   =  16,
    MANIA_6   =  32,
    MANIA_7   =  64,
    MANIA_8   =  128,
    MANIA_9   =  256,

    
};


#endif //KEYS_H
