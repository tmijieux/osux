#include <stdio.h>
#include <yaml.h>
#include <assert.h>

#include "yaml2.h"


enum state {
    TMAP,
    TLIST
};

int main(void)
{
    struct yaml_wrap *yamlw;
    
    parse_yaml_file(&yamlw, "description.yaml");
    return 0;
}

int parse_yaml_file(struct yaml_wrap *yamlw, const char *file_name)
{

    FILE *fh;
    yaml_parser_t parser;
    yaml_event_t  event;
    int done = 0;;
    struct list *list = NULL;
    struct hash_table *ht = NULL;
    int key = 1;
    char *keyname;
    struct stack *hst;
    enum state state;
    hst = stack_create();
    
    
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
		goto end;
		break;
		
	    case YAML_SEQUENCE_START_EVENT:
		stack_push(hst, list_new(0));
		state = TLIST;
		break;

	    case YAML_MAPPING_START_EVENT:
		ht = ht_create(100, NULL);
		if ( stack_size(hst) > 0 )
		{
		    ht_add_entry(stack_top(hst), key, new_ht);
		}
		stack_push(hst, ht);

		state = TMAP;
		break;
		
	    case YAML_MAPPING_END_EVENT:
	    case YAML_SEQUENCE_END_EVENT:
		stack_pop(hst);
		break;
        		
	    case YAML_SCALAR_EVENT:
		if (!key)
		{
		    keyname = strdup(event.data.scalar.value);
		    key = 1;
		}
		else
		{
		    if (type == TMAP)
			ht_add_entry(stack_top(hst), keyname,
				     strdup(event.data.scalar.value));
		    else if (type == TLIST)
			list_append(stack_top(hst),
				    strdup(event.data.scalar.value))
		    key = 0;
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



void* parse_yaml_file_rec(, enum state state)
{
    if (state == TMAP)
    {

    while (!done) {
	if (!yaml_parser_parse(&parser, &event)) {
	    printf("Yaml parser error %d\n", parser.error);
	    yaml_parser_delete(&parser);
	    return -1;
	}

	switch (event.type) {
	    case YAML_DOCUMENT_END_EVENT:
		done = 1;
		goto end;
		break;
		
	    case YAML_SEQUENCE_START_EVENT:
		stack_push(hst, list_new(0));
		state = TLIST;
		break;

	    case YAML_MAPPING_START_EVENT:
		ht = ht_create(100, NULL);
		if ( stack_size(hst) > 0 )
		{
		    ht_add_entry(stack_top(hst), key, new_ht);
		}
		stack_push(hst, ht);

		state = TMAP;
		break;
		
	    case YAML_MAPPING_END_EVENT:
	    case YAML_SEQUENCE_END_EVENT:
		stack_pop(hst);
		break;
        		
	    case YAML_SCALAR_EVENT:
		if (!key)
		{
		    keyname = strdup(event.data.scalar.value);
		    key = 1;
		}
		else
		{
		    if (type == TMAP)
			ht_add_entry(stack_top(hst), keyname,
				     strdup(event.data.scalar.value));
		    else if (type == TLIST)
			list_append(stack_top(hst),
				    strdup(event.data.scalar.value))
		    key = 0;
		}
		
		break;
	    default:
		break;
	}
    }
}



