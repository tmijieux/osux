#include <stdint.h>
#include <stdio.h>

#include "vosu-player.h"

G_DEFINE_TYPE(VosuPlayer, vosu_player, G_TYPE_OBJECT)

static void
message_cb(GstBus *bus, GstMessage *msg, VosuPlayer *player)
{
    (void) bus;
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error(msg, &err, &debug);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        gst_element_set_state(player->pipeline, GST_STATE_READY);
        break;
    }
    default:
        break;
    }
}

static void
vosu_player_init(VosuPlayer *p)
{
    p->pipeline = gst_element_factory_make("playbin", "playbin");
    p->bus = gst_element_get_bus(p->pipeline);
    gst_bus_add_signal_watch(p->bus);
    g_signal_connect(G_OBJECT(p->bus), "message",
                     G_CALLBACK(message_cb), p);
}

static void
vosu_player_finalize(GObject *obj)
{
    VosuPlayer *player = VOSU_PLAYER(obj);
    g_clear_pointer(&player->file, g_free);
    G_OBJECT_CLASS(vosu_player_parent_class)->finalize(obj);
    printf("player finalized\n");
}

static void
vosu_player_dispose(GObject *obj)
{
    VosuPlayer *player = VOSU_PLAYER(obj);
    gst_element_set_state(player->pipeline, GST_STATE_NULL);
    g_clear_object(&player->pipeline);
    g_clear_object(&player->bus);
    G_OBJECT_CLASS(vosu_player_parent_class)->dispose(obj);
    printf("player disposed\n");
}

static void
vosu_player_class_init(VosuPlayerClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = &vosu_player_dispose;
    G_OBJECT_CLASS(klass)->finalize = &vosu_player_finalize;
}

VosuPlayer *vosu_player_new(void)
{
    return VOSU_PLAYER(g_object_new(VOSU_TYPE_PLAYER, NULL));
}

void vosu_player_play(VosuPlayer *player)
{
    int err;
    err = gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    if (err == GST_STATE_CHANGE_FAILURE)
        g_printerr("Unable to set the pipeline to the playing state.\n");
    if (err == GST_STATE_CHANGE_ASYNC)
        g_printerr("will be played async.\n");

    printf("err=%d\n", err);
}

void vosu_player_pause(VosuPlayer *player)
{
    int err;
    err = gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
    if (err == GST_STATE_CHANGE_FAILURE)
        g_printerr("Unable to pause the pipeline.\n");
    if (err == GST_STATE_CHANGE_ASYNC)
        g_printerr("will be paused async.\n");
    printf("err=%d\n", err);
}

void
vosu_player_set_file(VosuPlayer *player, gchar const *filename)
{
    char *uri = g_strdup_printf("file://%s", filename);
    printf("file='%s'\n", filename);
    g_free(player->file);
    player->file = uri;
    g_object_set(player->pipeline, "uri", uri, NULL);
}

void
vosu_player_seek(VosuPlayer *player, int64_t offset)
{
    gst_element_seek_simple(
        player->pipeline, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, offset * GST_MSECOND);
}
