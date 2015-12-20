#include <stdio.h>
#include "../yaml2.h"

int main(int argc, char *argv[])
{
    struct yaml_wrap *yamlw;
    yaml2_parse_file(&yamlw, "description.yaml");
    yaml2_dump(stdout, yamlw);


    yaml2_free(yamlw);
    return 0;
}

