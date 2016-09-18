/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <yaml.h>
#include <stdbool.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "osux/error.h"
#include "osux/yaml.h"
#include "osux/stack.h"
#include "osux/compiler.h"

static void
hashtable_dump_cb(gpointer key_name, gpointer field, gpointer out_stream)
{
    fprintf(out_stream, "%s: ", (char*)key_name);
    osux_yaml_dump((FILE*) out_stream, (osux_yaml*) field);
}

void osux_yaml_dump(FILE *out, osux_yaml const *yaml)
{
    switch (yaml->type) {
    case OSUX_YAML_LIST:
	fprintf(out, _("yaml list start:\n"));
        GList *l = yaml->list;
        while (l != NULL) {
	    osux_yaml_dump(out, (osux_yaml*) l->data);
            l = l->next;
	}
	fprintf(out, _("yaml list end\n"));
	break;
    case OSUX_YAML_TABLE:
	fprintf(out, _("yaml table start:\n"));
	g_hash_table_foreach(yaml->table, hashtable_dump_cb, out);
	fprintf(out, _("yaml table end\n"));
	break;
    case OSUX_YAML_SCALAR:
	fprintf(out, "%s\n", yaml->scalar);
	break;
    default:
    case OSUX_YAML_INVALID:
        osux_warning(_("invalid yaml type encountered"));
        break;
    }
}

static void
insert_in_container(osux_yaml *cont, char const *keyname, osux_yaml *yaml)
{
    switch (cont->type) {
    case OSUX_YAML_TABLE:
        g_hash_table_insert(cont->table, g_strdup(keyname), yaml);
        break;
    case OSUX_YAML_LIST:
        cont->list = g_list_prepend(cont->list, yaml);
        break;
    default:
        osux_fatal(_("invalid container\n"));
        break;
    }
}

static osux_yaml*
wrap(osux_yaml_type type, gpointer value)
{
    osux_yaml *yaml = g_malloc(sizeof*yaml);
    yaml->type = type;
    yaml->value = value;
    return yaml;
}

static void
push_container(osux_stack *s, osux_yaml_type type,
               char const *keyname, void *value)
{
    osux_yaml *yaml = wrap(type, value);
    if (osux_stack_size(s) > 0) {
        osux_yaml *cont = osux_stack_head(s);
        insert_in_container(cont, keyname, yaml);
    }
    osux_stack_push(s, yaml);
}

#define CLEAR_STRING(s) do { g_clear_pointer(&(s), g_free); } while(0)

osux_yaml*
osux_yaml_new_from_file(char const *filepath)
{
    FILE *f;
    yaml_parser_t parser;
    osux_yaml *yaml = NULL, *tmp;
    osux_stack *container_stack;

    f = g_fopen(filepath, "r");
    if (f == NULL) {
	osux_error("%s: %s\n", filepath, strerror(errno));
	return NULL;
    }
    memset(&parser, 0, sizeof parser);
    if (!yaml_parser_initialize(&parser)) {
        fclose(f);
        osux_error(_("Failed to initialize yaml parser!\n"));
        yaml_parser_delete(&parser);
	return NULL;
    }
    yaml_parser_set_input_file(&parser, f);
    container_stack = osux_stack_new();

    bool done = false;
    char *keyname = NULL;
    while (!done) {
        GHashTable *table;
        yaml_event_t event;

        memset(&event, 0, sizeof event);
	if (!yaml_parser_parse(&parser, &event)) {
	    fprintf(stderr, _("Yaml parser error %d\n"), parser.error);
            yaml_event_delete(&event);
            break;
	}

	switch (event.type) {
	case YAML_DOCUMENT_END_EVENT:
        case YAML_STREAM_END_EVENT:
            done = true;
            break;
        case YAML_MAPPING_END_EVENT:
            if (osux_stack_size(container_stack))
                yaml = osux_stack_pop(container_stack);
            CLEAR_STRING(keyname);
            break;
	case YAML_SEQUENCE_END_EVENT:
            if (osux_stack_size(container_stack)) {
                yaml = osux_stack_pop(container_stack);
                yaml->list = g_list_reverse(yaml->list);
            }
            CLEAR_STRING(keyname);
	    break;
	case YAML_SEQUENCE_START_EVENT:
	    push_container(container_stack, OSUX_YAML_LIST, keyname, NULL);
            CLEAR_STRING(keyname);
	    break;
	case YAML_MAPPING_START_EVENT:
            table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify)osux_yaml_free);
	    push_container(container_stack, OSUX_YAML_TABLE, keyname, table);
            CLEAR_STRING(keyname);
	    break;
	case YAML_SCALAR_EVENT: {
            char *value = g_strdup((char*) event.data.scalar.value);
            if (osux_stack_size(container_stack) > 0) {
                osux_yaml *head = osux_stack_head(container_stack);
                if (head->type == OSUX_YAML_TABLE && keyname == NULL)
                    keyname = value;
                else {
                    tmp = wrap(OSUX_YAML_SCALAR, value);
                    insert_in_container(head, keyname, tmp);
                    CLEAR_STRING(keyname);
                }
	    } else { // the whole document IS a scalar:
                yaml = wrap(OSUX_YAML_SCALAR, value);
                CLEAR_STRING(keyname);
            }
	    break;
        }
	default:
	    break;
	}
	yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    while (osux_stack_size(container_stack))
        yaml = osux_stack_pop(container_stack);
    osux_stack_delete(container_stack);
    fclose(f);
    return yaml;
}

void osux_yaml_free(osux_yaml *yaml)
{
    if (yaml == NULL)
        return;

    switch (yaml->type) {
    case OSUX_YAML_TABLE:
        g_hash_table_destroy(yaml->table);
        break;
    case OSUX_YAML_LIST:
        g_list_free_full(yaml->list, (GDestroyNotify) osux_yaml_free);
        break;
    case OSUX_YAML_SCALAR:
        g_free(yaml->scalar);
        break;
    default:
    case OSUX_YAML_INVALID:
        osux_warning(_("invalid yaml type encountered"));
        break;
    }
    g_free(yaml);
}
