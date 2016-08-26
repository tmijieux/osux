#ifndef OSUX_LOCALE_H
#define OSUX_LOCALE_H

#ifdef __linux__
#define PUSH_C_LOCALE(var_id)                           \
    locale_t var_id = newlocale(LC_ALL_MASK, "C", 0);   \
    if (var_id == (locale_t) 0) {                       \
        perror("newlocale");                            \
        exit(EXIT_FAILURE);                             \
    }                                                   \
    locale_t old##var_id = uselocale(var_id);           \
    if (old##var_id == (locale_t) 0) {                  \
        perror("uselocale");                            \
        exit(EXIT_FAILURE);                             \
    }

#define POP_C_LOCALE(var_id)                            \
    locale_t ret##var_id = uselocale(old##var_id);      \
    if (ret##var_id == (locale_t) 0) {                  \
        perror("uselocale");                            \
        exit(EXIT_FAILURE);                             \
    }                                                   \
    freelocale(var_id);

#elif _WIN32

#define PUSH_C_LOCALE(var_id)                           \
    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);     \
    setlocale(LC_ALL, "C");                             \

#define POP_C_LOCALE(var_id)                            \
    _configthreadlocale(_DISABLE_PER_THREAD_LOCALE);    \
    setlocale(LC_ALL, "");                              \

#else

#define PUSH_C_LOCALE(var_id)
#define POP_C_LOCALE(var_id)

#endif


#endif // OSUX_LOCALE_H
