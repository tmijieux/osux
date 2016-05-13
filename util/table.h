#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>

struct table {
    int len;
    int max;
    const void ** t;
};

static inline struct table * table_new(int max)
{
    struct table * res = malloc(sizeof(*res));
    res->len = 0;
    res->max = max;
    res->t = malloc(sizeof(void *) * max);
    return res;
}

static inline void table_free(struct table * t)
{
    if(t == NULL)
	return;
    free(t->t);
    free(t);
}

static inline int table_len(const struct table * t)
{
    return t->len;
}

static inline int table_max(const struct table * t)
{
    return t->max;
}

static inline const void * table_get(const struct table * t, int i)
{
    return t->t[i];
}

static inline void table_add(struct table * t, const void * o)
{
    t->t[t->len] = o;
    t->len++;
}

static inline void table_set(struct table * t, int i, const void * o)
{
    t->t[i] = o;
}

#endif //TABLE_H
