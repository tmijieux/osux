#include "edosu-beatmap.h"
#include "edosu-adjust.h"
#include "osux/hitobject.h"

enum { COL_OFFSET = 0, COL_TYPE, COL_DETAILS, COL_NUM, };

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
                           COL_DETAILS, ho->details, -1);
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
                           COL_DETAILS, tp->details, -1);
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
                           COL_DETAILS, color, -1);
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
                       COL_DETAILS, detail, -1);
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
                           COL_DETAILS, "", -1);
    }
}

void edosu_beatmap_load_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm)
{
    GtkTreeStore *ts;
    ts = gtk_tree_store_new(COL_NUM, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

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
}
