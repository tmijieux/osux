#ifndef VOSU_APPLICATION_H
#define VOSU_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VOSU_TYPE_APPLICATION (vosu_application_get_type ())

G_DECLARE_FINAL_TYPE(VosuApplication, vosu_application,
                     VOSU, APPLICATION, GtkApplication)

#include "vosu-win.h"
#include "vosu-beatmap.h"
#include "vosu-replay.h"

struct _VosuApplication
{
    GtkApplication parent;
    VosuWindow *window;

    VosuBeatmap *beatmap;
    VosuReplay *replay;

    GtkFileFilter *osu_file_filter;
    GtkFileFilter *osr_file_filter;
    GtkFileFilter *all_file_filter;

};

VosuApplication *vosu_application_new(void);
gboolean vosu_application_open_beatmap(VosuApplication *app,
                                       gchar const *filename);
gboolean vosu_application_open_replay(VosuApplication *app,
                                      gchar const *filename);
void vosu_application_close_beatmap(VosuApplication *app);
void vosu_application_close_replay(VosuApplication *app);
void vosu_application_error(VosuApplication *app,
                            gchar const *title, gchar const *error);

G_END_DECLS

#endif // VOSU_APPLICATION_H
