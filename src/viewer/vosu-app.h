#ifndef VOSU_APPLICATION_H
#define VOSU_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VOSU_TYPE_APPLICATION (vosu_application_get_type ())

G_DECLARE_FINAL_TYPE(VosuApplication, vosu_application,
                     VOSU, APPLICATION, GtkApplication)

#include "vosu-beatmap.h"
#include "vosu-win.h"

struct _VosuApplication
{
    GtkApplication parent;
    VosuWindow *window;

    VosuBeatmap *beatmap;

    GtkFileFilter *osu_file_filter;
    GtkFileFilter *osr_file_filter;
    GtkFileFilter *all_file_filter;
};

VosuApplication *vosu_application_new(void);
void vosu_application_close_beatmap(VosuApplication *app);
void vosu_application_set_replay_file(VosuApplication *app,
                                      gchar const *filename);
G_END_DECLS

#endif // VOSU_APPLICATION_H
