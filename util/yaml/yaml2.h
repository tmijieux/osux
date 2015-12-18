#ifndef YAML2_H
#define YAML2_H

enum yaml_type {
    YAML_MAPPING = 10,
    YAML_SEQUENCE = 13,
    YAML_SCALAR = -9
};

union yaml_content {
    struct list *sequence;
    struct hash_table *mapping;
    char *scalar;
    void *value;
};

struct yaml_wrap {
    enum yaml_type type;
    union yaml_content content;
};

int yaml2_parse_file(struct yaml_wrap **yamlw, const char *file_name);

#endif //YAML2_H
