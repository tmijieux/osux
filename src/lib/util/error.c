#include "osux/error.h"

#define OSUX_ERROR_TO_STRING(error) [error] = #error,

static char const *osux_errmsg_[] = {
    OSUX_ERROR_LIST(OSUX_ERROR_TO_STRING)
};

char const *osux_errmsg(int error_code)
{
    return osux_errmsg_[error_code > 0 ? error_code : -error_code];
}
