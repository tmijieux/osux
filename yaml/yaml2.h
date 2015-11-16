
#ifndef YAML2_H
#define YAML2_H

enum yaml_type {
    YAML_MAPPING,
    YAML_SEQUENCE,
    YAML_SCALAR
};

union yaml_content {
    struct list *sequence;
    struct hash_table *mapping;
    char *scalar;
};

struct yaml_wrap {
    enum yaml_type type;
    union yaml_content content;
};

int parse_yaml_file(struct yaml_wrap **yamlw, const char *file_name);
    

#endif //YAML2_H
