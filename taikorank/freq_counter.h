#ifndef FREQ_COUNTER_H
#define FREQ_COUNTER_H

struct counter;

struct counter * cnt_new(void);
void cnt_free(struct counter * c);

void cnt_add(struct counter * c, void * data, const char * key, 
	     double val);

double cnt_get_total(struct counter * c);
double cnt_get_nb(struct counter * c, const char * key);
double cnt_get_nb_compressed(struct counter * c, const char * key,
			     int (*herit)(void *, void *));

void cnt_print(struct counter * c);
void cnt_print_compressed(struct counter * c,
			  int (*herit)(void *, void *));

#endif //FREQ_COUNTER_H
