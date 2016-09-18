#ifndef VOSU_SEQUENCE_H
#define VOSU_SEQUENCE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VOSU_TYPE_SEQUENCE (vosu_sequence_get_type())
G_DECLARE_FINAL_TYPE(VosuSequence, vosu_sequence,
                     VOSU, SEQUENCE, GObject);

struct _VosuSequence
{
    GObject parent;
    GSequence *gseq;

    gpointer data;
    GDestroyNotify free_data;
};

VosuSequence *vosu_sequence_new(GDestroyNotify data_destroy);
void vosu_sequence_add_additional_data(
    VosuSequence *seq, gpointer data, GDestroyNotify data_free);

#define vosu_sequence_append(seq, data)         \
    g_sequence_append((seq)->gseq, (data))

#define vosu_sequence_sort(seq, cmp, data)      \
    g_sequence_sort((seq)->gseq, (cmp), (data))

#define vosu_sequence_ref(seq)                          \
    VOSU_SEQUENCE(g_object_ref(G_OBJECT((seq))))

#define vosu_sequence_unref(seq)                \
    g_object_ref(G_OBJECT((seq)))

#define vosu_sequence_search(seq, data, cmp, cmp_data)          \
    g_sequence_search((seq)->gseq, (data), (cmp), (cmp_data))

#define vosu_sequence_get_begin_iter(seq)       \
    g_sequence_get_begin_iter((seq)->gseq)

#define vosu_sequence_get_length(seq)       \
    g_sequence_get_length((seq)->gseq)

G_END_DECLS

#endif //VOSU_SEQUENCE_H
