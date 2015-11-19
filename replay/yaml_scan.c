#include <yaml.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

struct pattern {
    double *d;
};
struct hash_table;
int ht_add_entry(struct hash_table*, const char*key, void*value);

int yaml_scan(struct hash_table *ht)
{
    int token_type;
    char *key;
    struct pattern *pat;
    FILE *file;
    yaml_parser_t parser;
    yaml_token_t token;
    int done = 0;
    int count = 0;
    int error = 0;

    file = fopen("description.yaml", "rb");
    assert(file);
    assert(yaml_parser_initialize(&parser));
    yaml_parser_set_input_file(&parser, file);

    while (!done)
    {
	if (!yaml_parser_scan(&parser, &token)) {
	    error = 1;
	    break;
	}

	switch (token.type) {
	    /* Token types (read before actual token) */
	    case YAML_KEY_TOKEN:
		pat = malloc(sizeof*pat);
	    case YAML_VALUE_TOKEN:
		token_type = token.type;
		break;
		    
	    case YAML_SCALAR_TOKEN:
		if ( token_type == YAML_KEY_TOKEN)
		{
		    key = strdup((char*) token.data.scalar.value);
		}
		else if ( token_type == YAML_VALUE_TOKEN )
		{
		    pat->d[count++] = atof((char*)token.data.scalar.value);
		}
		break;

	    case 11: // list end ']'
		ht_add_entry(ht, key, pat);
		break;
	    default:
		break;
	}
	done = (token.type == YAML_STREAM_END_TOKEN);
	yaml_token_delete(&token);
	count ++;
    }

    yaml_parser_delete(&parser);
    assert(!fclose(file));
    printf("%s (%d tokens)\n", (error ? "FAILURE" : "SUCCESS"), count);

    return 0;
}
