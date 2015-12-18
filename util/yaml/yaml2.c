#include <stdio.h>
#include <yaml.h>
#include <assert.h>

#include "yaml2.h"
#include "hashtable.h"
#include "list.h"
#include "stack.h"

void yaml2_dump(FILE *out, const struct yaml_wrap *yw);

static void mapping_dump(const char *name, void *data, void *args)
{
    fprintf(args, "%s: ", name);
    yaml2_dump(args, data);
}

void yaml2_dump(FILE *out, const struct yaml_wrap *yw)
{
    switch (yw->type) {
    case YAML_SEQUENCE:
	fprintf(out, "yaml sequence start:\n", yw->content.scalar);
	int si = list_size(yw->content.sequence);
	for (int i = 1; i <= si; ++i) {
	    fprintf(out, "- ");
	    yaml2_dump(out, list_get(yw->content.sequence, i));
	}
	fprintf(out, "yaml sequence end\n", yw->content.scalar);

	break;

    case YAML_MAPPING:
	fprintf(out, "yaml mapping start:\n", yw->content.scalar);
	ht_for_each(yw->content.mapping, mapping_dump, out);
	fprintf(out, "yaml mapping end\n", yw->content.scalar);
	break;

    case YAML_SCALAR:
	fprintf(out, "%s\n", yw->content.scalar);
	break;
    }
}

void parser_stack_push(struct stack *parser_stack,
		       enum yaml_type yt, void *value, char *keyname)
{
    struct yaml_wrap *newhead = calloc(sizeof*newhead, 1);
    newhead->type = yt;
    newhead->content.value = value;

    if (stack_size(parser_stack) > 0) {
	struct yaml_wrap *head = stack_peek(parser_stack);
	
	if (YAML_MAPPING == head->type)
	    ht_add_entry(head->content.mapping, keyname, newhead);
	else if (YAML_SEQUENCE == head->type) {
	    list_append(head->content.sequence, newhead);
	}
    }
    
    stack_push(parser_stack, newhead);
}

int yaml2_parse_file(struct yaml_wrap **yamlw, const char *file_name)
{
    FILE *fh;
    yaml_parser_t parser;
    yaml_event_t  event;
    int done = 0, key = 0;
    struct list *list = NULL;
    struct hash_table *ht = NULL;
    struct stack *parser_stack = stack_create(100);
    char *keyname = NULL;

//    parser_stack_push(parser_stack, YAML_SEQUENCE, list_new(0), NULL);
    
    assert( NULL != yamlw );
    fh = fopen(file_name, "r");
    
    if (!yaml_parser_initialize(&parser)) {
	return -1;
	fputs("Failed to initialize yaml parser!\n", stderr);
    }
    if (NULL == fh) {
	fprintf(stderr, "Failed to open file %s!\n", file_name);
	return -1;
    }
    
    yaml_parser_set_input_file(&parser, fh);

    while (!done) {
	if (!yaml_parser_parse(&parser, &event)) {
	    printf("Yaml parser error %d\n", parser.error);
	    yaml_parser_delete(&parser);
	    return -1;
	}

	switch (event.type) {
	case YAML_DOCUMENT_END_EVENT:
	    done = 1;
	    if (stack_size(parser_stack) > 0)
		*yamlw = stack_pop(parser_stack);
	    goto end;
	    break;
		
	case YAML_SEQUENCE_START_EVENT:
	    parser_stack_push(parser_stack, YAML_SEQUENCE, list_new(0), keyname);
	    break;

	case YAML_MAPPING_START_EVENT:
	    key = 0;
	    parser_stack_push(parser_stack, YAML_MAPPING,
			      ht_create(100, NULL), keyname);
	    break;
		
	case YAML_MAPPING_END_EVENT:
	case YAML_SEQUENCE_END_EVENT:
	    *yamlw = stack_pop(parser_stack);
	    break;
        		
	case YAML_SCALAR_EVENT:
	    if (stack_size(parser_stack) > 0) {
		struct yaml_wrap *head = stack_peek(parser_stack);
		if (head->type == YAML_MAPPING) {
		    if (!key)  {
			keyname = strdup(event.data.scalar.value);
			key = 1;
		    } else {
			parser_stack_push(parser_stack, YAML_SCALAR,
					  strdup(event.data.scalar.value), keyname);
			stack_pop(parser_stack);
			key = 0;
		    }
		} else {
		    assert(head->type == YAML_SEQUENCE);
		    parser_stack_push(parser_stack, YAML_SCALAR,
				      strdup(event.data.scalar.value), keyname);
		    stack_pop(parser_stack);
		}
	    } else {
		parser_stack_push(parser_stack, YAML_SCALAR,
				  strdup(event.data.scalar.value), NULL);
	    }
		
	    break;
	default:
	    break;
	}
	done =  (event.type == YAML_STREAM_END_EVENT);
      end:
	yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fh);    
    return 0;
}



