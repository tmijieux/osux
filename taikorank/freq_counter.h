#ifndef FREQ_COUNTER_H
#define FREQ_COUNTER_H

struct counter;

struct counter * cnt_new(void);
void cnt_free(struct counter * c);

void cnt_add(struct counter * c, void * data, char * key, double val);

double cnt_get_nb(struct counter * c, char * key);
double cnt_get_total(struct counter * c);

void cnt_print(struct counter * c);

#endif //FREQ_COUNTER_H
