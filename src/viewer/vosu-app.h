#ifndef VOSU_APPLICATION_H
#define VOSU_APPLICATION_H

#include <gtk/gtk.h>
#include "vosu-beatmap.h"

G_BEGIN_DECLS

#define VOSU_TYPE_APPLICATION (vosu_application_get_type ())

G_DECLARE_FINAL_TYPE(VosuApplication, vosu_application,
                     VOSU, APPLICATION, GtkApplication)

VosuApplication *vosu_application_new(void);
void vosu_application_close_beatmap(VosuApplication *app);

G_END_DECLS

#endif // VOSU_APPLICATION_H
