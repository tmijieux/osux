#include "vosu-sequence.h"

G_DEFINE_TYPE(VosuSequence, vosu_sequence, G_TYPE_OBJECT);

static void
vosu_sequence_init(VosuSequence *self)
{
    (void) self;
}

static void
vosu_sequence_finalize(GObject *obj)
{
    VosuSequence *seq = VOSU_SEQUENCE(obj);
    g_clear_pointer(&seq->gseq, g_sequence_free);
    // order matters, always free data after sequence
    if (seq->free_data != NULL)
        g_clear_pointer(&seq->data, seq->free_data);
    G_OBJECT_CLASS(vosu_sequence_parent_class)->finalize(obj);
}

static void
vosu_sequence_class_init(VosuSequenceClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = &vosu_sequence_finalize;
}

VosuSequence *vosu_sequence_new(GDestroyNotify data_destroy)
{
    VosuSequence *seq;
    seq = VOSU_SEQUENCE(g_object_new(VOSU_TYPE_SEQUENCE, NULL));
    seq->gseq = g_sequence_new(data_destroy);
    return seq;
}

void vosu_sequence_add_additional_data(
    VosuSequence *seq, gpointer data, GDestroyNotify free_data)
{
    seq->data = data;
    seq->free_data = free_data;
}
