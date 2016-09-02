#ifndef EDOSU_CIRCLE_H
#define EDOSU_CIRCLE_H

#include <stdlib.h>
#include <stdint.h>

struct vertex_data {
    float position[2];
};

extern struct vertex_data const edosu_circle_data[];
extern size_t edosu_circle_data_size;

extern uint8_t const edosu_circle_index[];
extern size_t edosu_circle_index_size;

#endif // EDOSU_CIRCLE_H
