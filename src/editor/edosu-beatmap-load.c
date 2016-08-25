#include "edosu-beatmap.h"
#include "edosu-adjust.h"
#include "osux/hitobject.h"

enum { COL_OFFSET = 0, COL_TYPE, COL_DETAILS, COL_OBJECT, COL_NUM };

static void
load_hit_objects(osux_beatmap *beatmap, GtkTreeStore *tree_store,
                 GtkTreeIter *hitobjects)
{
    for (unsigned i = 0; i < beatmap->hitobject_count; ++i)
    {
        osux_hitobject *ho = &beatmap->hitobjects[i];
        char *type;

        switch (HIT_OBJECT_TYPE(ho)) {
        case HITOBJECT_CIRCLE: type = _("Circle");  break;
        case HITOBJECT_SLIDER: type = _("Slider");  break;
        case HITOBJECT_SPINNER:type = _("Spinner");  break;
        case HITOBJECT_HOLD:   type = _("Hold");  break;
        default: type = _("Invalid type"); break;
        }

        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, hitobjects);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, ho->offset,
                           COL_TYPE, type,
                           COL_DETAILS, ho->details,
                           COL_OBJECT, ho, -1);
    }
}

static void
load_timing_points(osux_beatmap *beatmap,
                   GtkTreeStore *tree_store,
                   GtkTreeIter *timingpoints)
{
    for (unsigned i = 0; i < beatmap->timingpoint_count; ++i)
    {
        osux_timingpoint *tp = &beatmap->timingpoints[i];
        char *type;
        if (tp->inherited)
            type = _("Inherited");
        else
            type = _("Absolute");

        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, timingpoints);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, (int) tp->offset,
                           COL_TYPE, type,
                           COL_DETAILS, tp->details,
                           COL_OBJECT, tp, -1);
    }
}

static void
load_colors(osux_beatmap *beatmap, GtkTreeStore *tree_store, GtkTreeIter *colors)
{
    for (unsigned i = 0; i < beatmap->color_count; ++i)
    {
        osux_color *c = &beatmap->colors[i];
        GtkTreeIter iter;
        char color[50];
        sprintf(color, "%d,%d,%d", c->r, c->g, c->b);
        gtk_tree_store_append(tree_store, &iter, colors);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, c->id,
                           COL_TYPE, osux_color_type_get_name(c->type),
                           COL_DETAILS, color,
                           COL_OBJECT, c, -1);
    }
}

static void load_event(GtkTreeStore *tree_store, osux_event *event,
                       GtkTreeIter *parent_object)
{
    GtkTreeIter iter;
    char const *detail = osux_event_detail_string(event);

    gtk_tree_store_append(tree_store, &iter, parent_object);
    gtk_tree_store_set(tree_store, &iter,
                       COL_OFFSET, event->offset,
                       COL_TYPE, osux_event_type_get_name(event->type),
                       COL_DETAILS, detail,
                       COL_OBJECT, event, -1);
    for (unsigned i = 0; i < event->child_count; ++i)
        load_event(tree_store, event->childs[i], &iter);
}

static void
load_events(osux_beatmap *beatmap, GtkTreeStore *tree_store, GtkTreeIter *events)
{
    for (unsigned i = 0; i < beatmap->event_count; ++i) {
        if (!osux_event_is_object(&beatmap->events[i]))
            continue;
        load_event(tree_store, &beatmap->events[i], events);
    }
}

static void
load_bookmarks(osux_beatmap *beatmap, GtkTreeStore *tree_store,
               GtkTreeIter *bookmarks)
{
    for (unsigned i = 0; i < beatmap->bookmark_count; ++i)
    {
        int64_t b = beatmap->bookmarks[i];
        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, bookmarks);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, b,
                           COL_TYPE, _("Bookmark"),
                           COL_DETAILS, "",
                           COL_OBJECT, NULL, -1);
    }
}

void
edosu_beatmap_steal_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    beatmap->hitobjects = osux_bm->hitobjects;
    beatmap->events = osux_bm->events;
    beatmap->colors = osux_bm->colors;
    beatmap->timingpoints = osux_bm->timingpoints;
    
    osux_bm->hitobjects = NULL;
    osux_bm->hitobject_count = 0;
    osux_bm->events = NULL;
    osux_bm->event_count = 0;
    osux_bm->colors = NULL;
    osux_bm->color_count = 0;
    osux_bm->timingpoints = NULL;
    osux_bm->timingpoint_count = 0;
}

void edosu_beatmap_load_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeStore *ts;
    ts = gtk_tree_store_new(COL_NUM,
                            G_TYPE_INT, G_TYPE_STRING,
                            G_TYPE_STRING, G_TYPE_POINTER);
    beatmap->Objects = ts;
    gtk_tree_store_append(ts, &beatmap->TimingPoints, NULL);
    gtk_tree_store_set(ts, &beatmap->TimingPoints, COL_TYPE, _("TimingPoints"),
                       COL_OFFSET, osux_bm->timingpoint_count, -1);
    load_timing_points(osux_bm, ts, &beatmap->TimingPoints);

    gtk_tree_store_append(ts, &beatmap->HitObjects, NULL);
    gtk_tree_store_set(ts, &beatmap->HitObjects, COL_TYPE, _("HitObjects"),
                       COL_OFFSET, osux_bm->hitobject_count, -1);
    load_hit_objects(osux_bm, ts, &beatmap->HitObjects);

    gtk_tree_store_append(ts, &beatmap->Bookmarks, NULL);
    gtk_tree_store_set(ts, &beatmap->Bookmarks, COL_TYPE, _("Bookmarks"),
                       COL_OFFSET, osux_bm->bookmark_count, -1);
    load_bookmarks(osux_bm, ts, &beatmap->Bookmarks);

    gtk_tree_store_append(ts, &beatmap->Events, NULL);
    gtk_tree_store_set(ts, &beatmap->Events, COL_TYPE, _("Events"),
                       COL_OFFSET, osux_bm->event_count, -1);
    load_events(osux_bm, ts, &beatmap->Events);

    gtk_tree_store_append(ts, &beatmap->Colors, NULL);
    gtk_tree_store_set(ts, &beatmap->Colors, COL_TYPE, _("Colors"),
                       COL_OFFSET, osux_bm->color_count, -1);
    load_colors(osux_bm, ts, &beatmap->Colors);

    edosu_beatmap_steal_objects(beatmap, osux_bm);
}

static void
save_hitobjects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeIter hoiter;
    gboolean has_next;
    GtkTreeModel *tm = GTK_TREE_MODEL(beatmap->Objects);
    GValue value;
    memset(&value, 0, sizeof value);
    g_value_unset(&value); // "initialization"

    has_next = gtk_tree_model_iter_children(tm, &hoiter, &beatmap->HitObjects);
    while (has_next) {
        osux_hitobject ho, *ref;
        gtk_tree_model_get_value(tm, &hoiter, COL_OBJECT, &value);
        ref = g_value_get_pointer(&value);
        g_value_unset(&value);
        osux_hitobject_copy(ref, &ho);
        osux_beatmap_append_hitobject(osux_bm, &ho);
        has_next = gtk_tree_model_iter_next(tm, &hoiter);
    }
}

static void
save_timingpoints(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeIter tpiter;
    gboolean has_next;
    GtkTreeModel *tm = GTK_TREE_MODEL(beatmap->Objects);
    GValue value;
    memset(&value, 0, sizeof value);
    g_value_unset(&value); // "initialization"

    has_next = gtk_tree_model_iter_children(tm, &tpiter, &beatmap->TimingPoints);
    while (has_next) {
        osux_timingpoint tp, *ref;
        gtk_tree_model_get_value(tm, &tpiter, COL_OBJECT, &value);
        ref = g_value_get_pointer(&value);
        g_value_unset(&value);
        osux_timingpoint_copy(ref, &tp);
        osux_beatmap_append_timingpoint(osux_bm, &tp);
        has_next = gtk_tree_model_iter_next(tm, &tpiter);
    }
}

static void
save_events(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeIter eviter;
    gboolean has_next;
    GtkTreeModel *tm = GTK_TREE_MODEL(beatmap->Objects);
    GValue value;
    memset(&value, 0, sizeof value);
    g_value_unset(&value); // "initialization"

    has_next = gtk_tree_model_iter_children(tm, &eviter, &beatmap->Events);
    while (has_next) {
        osux_event ev, *ref;
        gtk_tree_model_get_value(tm, &eviter, COL_OBJECT, &value);
        ref = g_value_get_pointer(&value);
        g_value_unset(&value);
        osux_event_copy(ref, &ev);
        osux_beatmap_append_event(osux_bm, &ev);
        has_next = gtk_tree_model_iter_next(tm, &eviter);
    }
}

static void
save_colors(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeIter coliter;
    gboolean has_next;
    GtkTreeModel *tm = GTK_TREE_MODEL(beatmap->Objects);
    GValue value;
    memset(&value, 0, sizeof value);
    g_value_unset(&value); // "initialization"

    has_next = gtk_tree_model_iter_children(tm, &coliter, &beatmap->Colors);
    while (has_next) {
        osux_color col, *ref;
        gtk_tree_model_get_value(tm, &coliter, COL_OBJECT, &value);
        ref = g_value_get_pointer(&value);
        g_value_unset(&value);
        osux_color_copy(ref, &col);
        osux_beatmap_append_color(osux_bm, &col);
        has_next = gtk_tree_model_iter_next(tm, &coliter);
    }
}

void
edosu_beatmap_objects_save(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    save_hitobjects(beatmap, osux_bm);
    save_timingpoints(beatmap, osux_bm);
    save_events(beatmap, osux_bm);
    save_colors(beatmap, osux_bm);
}
