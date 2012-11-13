#include <cutils/properties.h>

#include <sys/system_properties.h>




int GetSystemSettingString(const char *path, char *value, char *defaultv)
{
    return property_get(path, value, defaultv);
}


