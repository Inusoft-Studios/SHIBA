#include "platform/version.h"

#define SHIBA_STR_IMPL(x) #x
#define SHIBA_STR(x) SHIBA_STR_IMPL(x)

const char* shiba::versionStr() {
    return "shiba " SHIBA_STR(SHIBA_VERSION_MAJOR)
           "."      SHIBA_STR(SHIBA_VERSION_MINOR)
           "."      SHIBA_STR(SHIBA_VERSION_PATCH);
}