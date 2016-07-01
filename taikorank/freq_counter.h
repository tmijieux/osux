#ifndef FREQ_COUNTER_H
#define FREQ_COUNTER_H

struct counter;
typedef double (*herit_fun)(const void*, const void*);

struct counter * cnt_new(void);
void cnt_free(struct counter * c);

void cnt_add(struct counter * c, const void * data, const char * key, 
	     double val);

double cnt_get_total(const struct counter * c);
double cnt_get_total_compressed(const struct counter * c,
				herit_fun herit);

double cnt_get_nb(const struct counter * c, const char * key);
double cnt_get_nb_compressed(const struct counter * c,
			     const char * key, herit_fun herit);

void cnt_print(const struct counter * c);
void cnt_print_compressed(const struct counter * c, herit_fun herit);

#endif //FREQ_COUNTER_H
