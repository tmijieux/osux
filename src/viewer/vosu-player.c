#include <stdint.h>
#include <stdio.h>

#include "vosu-player.h"

G_DEFINE_TYPE(VosuPlayer, vosu_player, G_TYPE_OBJECT)

static void
finished_cb(VosuPlayer *player)
{
    vosu_player_pause(player);
}

static void
message_cb(GstBus *bus, GstMessage *msg, VosuPlayer *player)
{
    (void) bus;
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error(msg, &err, &debug);
        g_print("music player error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        gst_element_set_state(player->pipeline, GST_STATE_READY);
        break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
        //the following allow to seek in the music before it was even started
        if (GST_ELEMENT(msg->src) == player->pipeline) {
            GstState old_state, new_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
            if (new_state == GST_STATE_PLAYING &&
                player->early_seek_offset)
            {
                vosu_player_seek(player, player->early_seek_offset);
                player->early_seek_offset = 0;
            }
        }
        break;
    }
    default:
        break;
    }
}

static void
newpad_cb(GstElement *decodebin, GstPad *pad, VosuPlayer *player)
{
    (void) decodebin;
    GstCaps *caps;
    GstStructure *str;
    GstPad *audiopad;

    /* only link once */
    audiopad = gst_element_get_static_pad(player->snk, "sink");
    if (GST_PAD_IS_LINKED(audiopad)) {
        g_object_unref(audiopad);
        return;
    }

    /* check media type */
    caps = gst_pad_query_caps(pad, NULL);
    str = gst_caps_get_structure(caps, 0);
    if (!g_strrstr(gst_structure_get_name(str), "audio")) {
        gst_caps_unref(caps);
        gst_object_unref(audiopad);
        return;
    }
    gst_caps_unref(caps);

    /* link'n'play */
    gst_pad_link(pad, audiopad);
    g_object_unref(audiopad);
}

static void
vosu_player_init(VosuPlayer *p)
{
    p->pipeline = gst_pipeline_new("pipeline");
    p->src = gst_element_factory_make("filesrc", NULL);
    p->filt = gst_element_factory_make("decodebin", NULL);
    p->snk = gst_element_factory_make("autoaudiosink", NULL);
    p->query = gst_query_new_position(GST_FORMAT_TIME);
    g_signal_connect(G_OBJECT(p->filt), "pad-added",
                     G_CALLBACK(newpad_cb), p);
    gst_bin_add_many(GST_BIN(p->pipeline),
                     p->src, p->filt, p->snk, NULL);
    gst_element_link(p->src, p->filt);

    p->bus = gst_element_get_bus(p->pipeline);
    gst_bus_add_signal_watch(p->bus);
    g_signal_connect(p->bus, "message",
                     G_CALLBACK(message_cb), p);
    g_signal_connect_swapped(p->filt, "drained",
                             G_CALLBACK(finished_cb), p);
}

static void
vosu_player_finalize(GObject *obj)
{
    VosuPlayer *player = VOSU_PLAYER(obj);
    g_clear_pointer(&player->file, g_free);
    G_OBJECT_CLASS(vosu_player_parent_class)->finalize(obj);
}

int64_t vosu_player_get_time(VosuPlayer *player)
{
    GstFormat fmt = GST_FORMAT_TIME;
    int64_t current = -1;
    gst_element_query(player->pipeline, player->query);
    gst_query_parse_position(player->query, &fmt, &current);
    //gst_element_query_position(player->pipeline, fmt, &current);
    return current;
}

static void
vosu_player_dispose(GObject *obj)
{
    VosuPlayer *player = VOSU_PLAYER(obj);
    gst_element_set_state(player->pipeline, GST_STATE_NULL);
    g_clear_object(&player->pipeline);
    g_clear_object(&player->bus);
    g_clear_pointer(&player->query, gst_query_unref);
    G_OBJECT_CLASS(vosu_player_parent_class)->dispose(obj);
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
}

void vosu_player_pause(VosuPlayer *player)
{
    int err;
    err = gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
    if (err == GST_STATE_CHANGE_FAILURE)
        g_printerr("Unable to pause the pipeline.\n");
}

void
vosu_player_set_file(VosuPlayer *player, gchar const *filename)
{
    //char *uri = g_strdup_printf("file://%s", filename);
    printf("music file='%s'\n", filename);
    g_free(player->file);
    player->file = g_strdup(filename);
    g_object_set(player->src, "location", player->file, NULL);
    gst_element_set_state(player->pipeline, GST_STATE_READY);
}

void
vosu_player_seek(VosuPlayer *player, int64_t offset)
{
    if (!gst_element_seek_simple(
        player->pipeline, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
        offset * GST_MSECOND))
    {
        player->early_seek_offset = offset;
        return;
    }
}
