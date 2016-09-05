#include "osux/hit.h"
#include "osux/error.h"
#include "osux/keys.h"
#include "osux/replay.h"
#include "osux/beatmap.h"

static unsigned base_window[] = {
    [HIT_RAINBOW_300] = 50,  // guess
    [HIT_300]         = 80,
    [HIT_200]         = 110, // guess
    [HIT_100]         = 140,
    [HIT_50]          = 200,
    [HIT_MISS]        = 370, // guess
};

static unsigned step_window[] = {
    [HIT_RAINBOW_300] = 5, // guess
    [HIT_300]         = 6,
    [HIT_200]         = 7, // guess
    [HIT_100]         = 8,
    [HIT_50]          = 10,
    [HIT_MISS]        = 12,
};

char const *hit_str[] = {
    [HIT_RAINBOW_300] = "MAX",
    [HIT_50] = "50",
    [HIT_100] = "100",
    [HIT_200] = "200",
    [HIT_300] = "300",
    [HIT_MISS] = "miss",
};

static double base_hit_window(
    double overall_difficulty, double base_window, double window_step)
{
    return base_window - window_step * overall_difficulty;
}

#define TWO_THIRD  (.666666666666666)
#define FOUR_THIRD (1.33333333333333)

int osux_get_approach_time(double ar, int mods)
{
    double step = 120.0, base = 1800.0;

    if (mods & MOD_EASY)
        ar /= 2;
    else if (mods & MOD_HARDROCK)
        ar = min(10., 1.4 * ar);

    if (likely( ar > 5.0 )) {
        ar -= 5;
        step = 150.0;
        base = 1200.0;
    }

    double time = base - ar * step;
    if (mods & MOD_DOUBLETIME)
        time *= TWO_THIRD;
    else if (mods & MOD_HALFTIME)
        time *= FOUR_THIRD;

    return (int) time;
}

void osux_get_hit_windows(
    double window[], // Array of size MAX_HIT_TYPE to be filled by this function
    double od, // map overall difficulty
    int mods) // game mods, only EZ,HR,,HT,DT(or NC) have influence on hit window
{
    if (mods & MOD_EASY)
        od /= 2.;
    else if (mods & MOD_HARDROCK)
        od = min(10., 1.4 * od);

    memset(window, 0, sizeof window[0] * MAX_HIT_TYPE);

    for (unsigned iHit = 0; iHit < MAX_HIT_TYPE; ++iHit) {
        window[iHit] = base_hit_window(
            od, base_window[iHit], step_window[iHit]);

        if (mods & MOD_DOUBLETIME)
            window[iHit] *= TWO_THIRD;
        else if (mods & MOD_HALFTIME)
            window[iHit] *= FOUR_THIRD;
    }
}

static void print_hit_windows(double window[])
{
    for (unsigned i = 0; i < MAX_HIT_TYPE; ++i)
        printf("hit_window '%s': %g\n", hit_str[i], window[i]);
}


#define HIT_KEY_DIFF(d1, d2)                                    \
    (HIT_KEY_PRESSED((d1)->keys) ^ HIT_KEY_PRESSED((d2)->keys))

static int get_hit_type_std(double window[], int64_t distance)
{
    distance = labs(distance);

    if (distance < window[HIT_300])
        return HIT_300;
    else if (distance < window[HIT_100])
        return HIT_100;
    else if (distance < window[HIT_50])
        return HIT_50;
    else
        return HIT_MISS;
}

static int get_hit_type_taiko(double window[], int64_t distance)
{
    distance = labs(distance);

    if (distance < window[HIT_300])
        return HIT_300;
    else if (distance < window[HIT_50])
        return HIT_100;
    else
        return HIT_MISS;
}

static inline int64_t time_distance(osux_keypress *kp, osux_hitobject *ho)
{
    return (int64_t) kp->offset - ho->offset;
}

static int cmp_time_offset(void const *a, void const *b)
{
    return ((osux_replay_data const *)a)->time_offset -
        ((osux_replay_data const*)b)->time_offset;
}

static void print_time_offset_human_readable(int64_t offset)
{
    int64_t minutes, seconds, milliseconds;
    milliseconds = offset % 1000;
    offset /= 1000;
    seconds = offset % 60;
    minutes = offset / 60;
    printf("%ld: %ldm %lds.%03ld", offset*1000+milliseconds,
           minutes, seconds, milliseconds);
}

// return value's nth set bit, all other bits are zeroed
static uint32_t get_nth_bit_set(uint32_t value, unsigned n)
{
    unsigned shift = 0;
    while (n > 0) {

        if (value & 1)
            n--;
        value >>= 1;
        ++ shift;
    }
    return 1U << (shift-1);
}

static unsigned bit_set_count(uint32_t value)
{
    unsigned count = 0;
    for (unsigned i = 0; i < 32; ++i)
        if ((value >> i) & 1)
            ++ count;
    return count;
}

// remove double keypress in standart difficulties replay
// these double appear because keyboard hit key will also flag as
// mouse hit key (this is the reason why you cant click with mouse when
// your mouse keys are pressed
static void keypress_remove_doubles(osux_hits *h)
{
    if ((h->game_mode != GAME_MODE_STD && h->game_mode != GAME_MODE_TAIKO)
        || !h->keypress_count)
        return;

    osux_keypress *unique_kp;
    size_t unique_kp_size;
    osux_keypress *kp = h->keypress;

    ALLOC_ARRAY(unique_kp, unique_kp_size, h->keypress_count);
    unique_kp_size = 1;
    // copy the first one
    memcpy(&unique_kp[0], &kp[0], sizeof*kp);

    for (unsigned i = 1; i < h->keypress_count; ++i) {
        unsigned or_key = kp[i].key | kp[i-1].key;
        if ((kp[i].offset == kp[i-1].offset) &&
            (kp[i].release == kp[i-1].release) &&
            (or_key == KEY_LEFT || or_key == KEY_RIGHT)) {
            // skip if this is the same keypress
            continue;
        }
        memcpy(&unique_kp[unique_kp_size], &kp[i], sizeof*kp);
        ++ unique_kp_size;
    }

    g_free(h->keypress);
    h->keypress = unique_kp;
    h->keypress_count = unique_kp_size;
}

static int osux_hits_compute_data(
    osux_hits *h, osux_replay_data const *data, size_t data_count)
{
    ALLOC_ARRAY(h->data, h->data_count, data_count-2);
    COPY_ARRAY(h->data+1, data+2, data_count-3); // evict first, second and last
    qsort(h->data+1, h->data_count-1, sizeof*data, &cmp_time_offset);
    memset(h->data, 0, sizeof*data); // zero first element
    return 0;
}

static int osux_hits_compute_keypress(osux_hits *h)
{
    size_t keypress_cap;
    osux_replay_data *data = h->data;
    h->keypress_count = 0;
    ALLOC_ARRAY(h->keypress, keypress_cap, 500);

    for (unsigned i = 1; i < h->data_count; ++i) {
        int keys_diff = HIT_KEY_DIFF(&data[i], &data[i-1]);
        if (keys_diff != 0) {
            unsigned key_count = bit_set_count(keys_diff);
            HANDLE_ARRAY_SIZE(h->keypress,
                              h->keypress_count+key_count,
                              keypress_cap);
            for (unsigned k = 1; k <= key_count; ++k) {

                osux_keypress *kp = &h->keypress[h->keypress_count];
                kp->offset = data[i].time_offset;
                kp->key = get_nth_bit_set(keys_diff, k);
                if (kp->key & data[i].keys)
                    kp->release = false;
                else
                    kp->release = true;
                ++ h->keypress_count;
            }
        }
    }
    keypress_remove_doubles(h);
    return 0;
}

void osux_hits_print_keypress(osux_hits *h)
{
    for (unsigned i = 0; i < h->keypress_count; ++i) {
        osux_keypress *kp = &h->keypress[i];
        print_time_offset_human_readable(kp->offset);
        printf(" | key=%u -- %s\n", kp->key, kp->release ? "RELEASE" : "PRESS");
    }
}

// basic hit computation for standard mode:
// all spinner / slider are free 300's
// only timing is currently taken into account; no position
static int osux_hits_compute_hits_std_basic(
    osux_hits *h, osux_beatmap const *beatmap)
{
    double window[MAX_HIT_TYPE];
    osux_keypress *p_keypress = h->keypress;
    osux_keypress *keypresses_end = h->keypress + h->keypress_count;
    osux_hitobject *p_hitobject;
    osux_hitobject *hitobjects_end = beatmap->hitobjects + beatmap->hitobject_count;
    int i_hitobject;
    int64_t max_distance = 0;

    osux_get_hit_windows(window, h->overall_difficulty, h->mods);
    ALLOC_ARRAY(h->hits, h->hits_size, beatmap->hitobject_count);

    for ( p_hitobject = &beatmap->hitobjects[0], i_hitobject = 0;
          p_hitobject != hitobjects_end;
          ++ p_hitobject, ++ i_hitobject   )
    {
        int64_t distance = time_distance( p_keypress, p_hitobject );
        max_distance = max(max_distance, labs(distance));

        // get first key press in (miss) hit window
        while ((p_keypress != keypresses_end && p_keypress->release)
               || distance < -window[HIT_MISS])
        {
            ++ p_keypress;
            distance = time_distance( p_keypress, p_hitobject );
        }

        // free win spinner
        if (HIT_OBJECT_IS_SPINNER(p_hitobject))
        {
            h->hits[i_hitobject].hit_type = HIT_300;
            h->hits[i_hitobject].hitted = true;

            while (p_keypress != keypresses_end &&
                   p_keypress->offset < p_hitobject->end_offset)
            {
                // get out of spinner
                ++ p_keypress;
            }
            continue;
        }

        // if no keypress in the hit window, that's a miss
        if (distance > window[HIT_50]) {
            h->hits[i_hitobject].hitted = false;
            h->hits[i_hitobject].hit_type = HIT_MISS;

            if (distance < window[HIT_MISS] && HIT_OBJECT_IS_SLIDER(p_hitobject))
            {
                h->hits[i_hitobject].hit_type = HIT_300;
                h->hits[i_hitobject].hitted = true;
                ++ p_keypress;
                continue;
            }
            printf("missed 'after' distance=%ld, hit_offset=%ld, ho_offset=%ld\n",
                   distance, p_keypress->offset, p_hitobject->offset);
        } else {
            h->hits[i_hitobject].hitted = true;
            if (HIT_OBJECT_IS_CIRCLE(p_hitobject))
                h->hits[i_hitobject].hit_type = get_hit_type_std(window, distance);
            else
                // free win slider
                h->hits[i_hitobject].hit_type = HIT_300;

            printf("hit %s distance=%ld, hit_offset=%ld, ho_offset=%ld\n",
                   hit_str[h->hits[i_hitobject].hit_type], distance,
                   p_keypress->offset, p_hitobject->offset);
            ++ p_keypress;
        }
    }

    print_hit_windows(window);
    printf("OD: %g\n", beatmap->OverallDifficulty);
    printf("max_distance: %ld\n", max_distance);
    return 0;
}

// basic hit computation for taiko mode:
// drum roll(slider) is ignored
// shaker (spinner) is free win
static int osux_hits_compute_hits_taiko_basic(
    osux_hits *h, osux_beatmap const *beatmap)
{
    double window[MAX_HIT_TYPE];
    osux_keypress *p_keypress = h->keypress;
    osux_keypress *keypresses_end = h->keypress + h->keypress_count;
    osux_hitobject *p_hitobject;
    osux_hitobject *hitobjects_end = beatmap->hitobjects + beatmap->hitobject_count;
    int i_hitobject;
    int64_t max_distance = 0;

    osux_get_hit_windows(window, h->overall_difficulty, h->mods);
    ALLOC_ARRAY(h->hits, h->hits_size, beatmap->hitobject_count);

    for ( p_hitobject = &beatmap->hitobjects[0], i_hitobject = 0;
          p_hitobject != hitobjects_end;
          ++ p_hitobject, ++ i_hitobject   )
    {
        int64_t distance = time_distance( p_keypress, p_hitobject );

        // get first key press in (miss) hit window
        while ((p_keypress != keypresses_end && p_keypress->release)
               || distance < -window[HIT_50])
        {
            ++ p_keypress;
            distance = time_distance( p_keypress, p_hitobject );
        }


        // free win shaker/spinner
        if (HIT_OBJECT_IS_SPINNER(p_hitobject) || HIT_OBJECT_IS_SLIDER(p_hitobject))
        {
            h->hits[i_hitobject].hit_type = HIT_300;
            h->hits[i_hitobject].hitted = true;

            while (p_keypress != keypresses_end &&
                   p_keypress->offset < p_hitobject->end_offset)
            {
                // get out of shaker/spinner
                ++ p_keypress;
            }
            continue;
        }

        // if no keypress in the hit window, that's a miss
        if (distance > window[HIT_50]) {
            h->hits[i_hitobject].hitted = false;
            h->hits[i_hitobject].hit_type = HIT_MISS;
            printf("hit %s distance=%ld, hit_offset=%ld, ho_offset=%ld\n",
                   hit_str[h->hits[i_hitobject].hit_type], distance,
                   p_keypress->offset, p_hitobject->offset);
        } else {
            h->hits[i_hitobject].hitted = true;
            if (HIT_OBJECT_IS_CIRCLE(p_hitobject)) {
                h->hits[i_hitobject].hit_type =
                    get_hit_type_taiko(window, distance);
            } else
                // free win slider
                h->hits[i_hitobject].hit_type = HIT_300;
            printf("hit %s distance=%ld, hit_offset=%ld, ho_offset=%ld\n",
                   hit_str[h->hits[i_hitobject].hit_type], distance,
                   p_keypress->offset, p_hitobject->offset);

            max_distance = max(max_distance, labs(distance));
            ++ p_keypress;
        }
    }

    print_hit_windows(window);
    printf("OD: %g\n", beatmap->OverallDifficulty);
    printf("max_distance: %ld\n", max_distance);
    return 0;
}

static void compute_hit_stats(int hit_stats[], osux_hit *hits, size_t hits_size)
{
    memset(hit_stats, 0, sizeof hit_stats[0] * MAX_HIT_TYPE);

    for (unsigned i = 0; i < hits_size; ++i)
        ++ hit_stats[hits[i].hit_type];
}

static void print_hit_stats(int hit_stats[])
{
    printf("300: %d\n", hit_stats[HIT_300]);
    printf("100: %d\n", hit_stats[HIT_100]);
    printf("50: %d\n", hit_stats[HIT_50]);
    printf("MISS: %d\n", hit_stats[HIT_MISS]);
}

int osux_hits_free(osux_hits *h)
{
    g_free(h->data);
    g_free(h->keypress);
    g_free(h->hits);

    return 0;
}

typedef int (*compute_hit_fn)(osux_hits*, osux_beatmap const*);
compute_hit_fn compute_mode[] = {
    [GAME_MODE_STD] = osux_hits_compute_hits_std_basic,
    [GAME_MODE_TAIKO] = osux_hits_compute_hits_taiko_basic,
    [GAME_MODE_CTB] = NULL,
    [GAME_MODE_MANIA] = NULL,
};

int osux_hits_init(osux_hits *hits,
                   osux_beatmap const *beatmap, osux_replay const *replay)
{
    memset(hits, 0, sizeof*hits);
    hits->game_mode = replay->game_mode;
    hits->overall_difficulty = beatmap->OverallDifficulty;
    hits->mods = replay->mods;

    if (replay->game_mode >= MAX_GAME_MODE)
        return -OSUX_ERR_INVALID_GAME_MODE;
    if (replay->game_mode != beatmap->Mode)
        return -OSUX_ERR_AUTOCONVERT_NOT_SUPPORTED;

    osux_hits_compute_data(hits, replay->data, replay->data_count);
    osux_hits_compute_keypress(hits);
    osux_hits_print_keypress(hits);

    if (hits->keypress == NULL) {
        osux_hits_free(hits);
        return -1;
    }

    compute_hit_fn compute = compute_mode[hits->game_mode];
    if (compute == NULL) {
        osux_hits_free(hits);
        return -OSUX_ERR_GAME_MODE_NOT_SUPPORTED;
    }

    (*compute)(hits, beatmap);

    if (hits->hits == NULL) {
        osux_hits_free(hits);
        return -OSUX_ERR_UNKNOWN_ERROR;
    }

    int hit_stats[MAX_HIT_TYPE];
    compute_hit_stats(hit_stats, hits->hits, hits->hits_size);
    print_hit_stats(hit_stats);

    return 0;
}
