#ifndef OSUX_LOCALE_H
#define OSUX_LOCALE_H

#include <locale.h>

#ifdef __linux__
#define SET_THREAD_LOCALE(var_id_, localestr_)                          \
    locale_t new_##var_id_ = newlocale(LC_ALL_MASK, localestr_, 0);     \
    if (new_##var_id_ == (locale_t) 0) {                                \
        perror("newlocale");                                            \
        exit(EXIT_FAILURE);                                             \
    }                                                                   \
    locale_t old_##var_id_ = uselocale(new_##var_id_);                  \
    if (old_##var_id_ == (locale_t) 0) {                                \
        perror("uselocale");                                            \
        exit(EXIT_FAILURE);                                             \
    }

#define RESTORE_THREAD_LOCALE(var_id_)                  \
    locale_t ret_##var_id_ = uselocale(old_##var_id_);  \
    if (ret_##var_id_ == (locale_t) 0) {                \
        perror("uselocale");                            \
        exit(EXIT_FAILURE);                             \
    }                                                   \
    freelocale(new_##var_id_);

#elif _WIN32

#define SET_THREAD_LOCALE(var_id, localestr_)                   \
    do {                                                        \
        _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);         \
        setlocale(LC_ALL, localestr_);                          \
        _configthreadlocale(_DISABLE_PER_THREAD_LOCALE);        \
    } while(0)

#else

#define SET_THREAD_LOCALE(var_id, localestr_)
#define POP_LOCALE(var_id, localestr_)

#endif


#endif // OSUX_LOCALE_H
