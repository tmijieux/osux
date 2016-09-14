#ifndef VOSU_PLAYER_H
#define VOSU_PLAYER_H

#include <gst/gst.h>

G_BEGIN_DECLS

#define VOSU_TYPE_PLAYER (vosu_player_get_type())

G_DECLARE_FINAL_TYPE(VosuPlayer, vosu_player, VOSU, PLAYER, GObject);

struct _VosuPlayer
{
    GObject parent;
    GstElement *pipeline;

    GstElement *src;
    GstElement *filt;
    GstElement *snk;
    GstBus *bus;

    gchar *file;
    int64_t early_seek_offset;
};

VosuPlayer *vosu_player_new(void);

void vosu_player_play(VosuPlayer *player);
void vosu_player_pause(VosuPlayer *player);
void vosu_player_seek(VosuPlayer *player, int64_t offset);
void vosu_player_set_file(VosuPlayer *player, gchar const *filename);
guint64 vosu_player_get_time(VosuPlayer *player);

G_END_DECLS

#endif // VOSU_PLAYER_H
