#ifndef VOSU_SEQUENCE_H
#define VOSU_SEQUENCE_H

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>
#include "vosu-player.h"
#include "vosu-render.h"

G_BEGIN_DECLS

#define VOSU_TYPE_SEQUENCE (vosu_sequence_get_type())

G_DECLARE_FINAL_TYPE(VosuSequence, vosu_sequence,
                     VOSU, SEQUENCE, GObject);

struct _VosuSequence
{
    GObject parent;

};

G_END_DECLS

#endif //VOSU_SEQUENCE_H
