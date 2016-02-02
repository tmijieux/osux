

create table beatmap_set
(
        beatmap_set_id      int primary key not null,
        osu_beatmap_set_id  int,
        creator             text,
        artist              text,
        artist_unicode      text,
        title               text,
        title_unicode       text,
        display_font        text,
        tags                text,
        source              text,
        folder              text,
        status              int
);

create table beatmap
(
        beatmap_id      integer primary key not null,
        osu_beatmap_id  int,
        osu_forum_thrd  int,
        beatmap_set_id  int,

        game_mode       int,

        audio_filename  text,
        diff_name       text,
        md5_hash        text,
        osu_filename    text,
        
        circles         int,
        sliders         int,
        spinners        int,
        last_modification       text,
        last_checked            text,
        
        approach_rate   real,
        circle_size     real,
        hp_drain        real,
        overall_diff    real,
        slider_velocity real,
        stack_leniency  real,
        
        drain_time      int,
        total_time      int,
        preview_time    int,
        
        bpm_avg         int,
        bpm_max         int,
        bpm_min         int,

        local_offset    int,
        online_offset   int,

        already_played  int,
        last_played     text,
        
        ignore_hitsound int,
        ignore_skin     int,
        disable_sb      int,
        disable_video   int,
        visual_override int,
        mania_scroll_speed      int,
        foreign key(beatmap_set_id) references beatmap_set(beatmap_set_id)  
);


