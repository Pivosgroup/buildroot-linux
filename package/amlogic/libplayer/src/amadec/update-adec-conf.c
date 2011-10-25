#include <ctype.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINE_SIZE           128
#define CONFIG_FILE_NAME    "amadec.conf"


int get_key(char **line, char **key, char **value)
{
    char *linepos;
    char *temp;

    linepos = *line;
    if (linepos[0] == NULL || linepos[0] == '\0') {
        return -1;
    }

    while (isspace(linepos[0])) {
        linepos++;
    }

    if (linepos[0] == '\0') {
        return -1;
    }

    *key = linepos;

    for (;;) {
        linepos++;
        if (linepos[0] == '\0') {
            return -1;
        }
        if (isspace(linepos[0])) {
            break;
        }
        if (linepos[0] == '=') {
            break;
        }
    }

    temp = linepos;

    while (isspace(linepos[0])) {
        linepos++;
    }

    if (linepos[0] == '=') {
        linepos++;
    } else {
        return -1;
    }

    temp[0] = '\0';

    while (isspace(linepos[0])) {
        linepos++;
    }

    if (linepos[0] == '\0') {
        return -1;
    }

    if (linepos[0] == '"') {
        linepos++;
    } else {
        return -1;
    }

    *value = linepos;

    temp = strchr(linepos, '"');
    if (!temp) {
        return -1;
    }

    temp[0] = '\0';
    temp++;

    *line = temp;

    return 0;
}

int update_firmware_config(void)
{
    char *key;
    char *value;
    char *tmp;
    char *line[MAX_LINE_SIZE];
    FILE *fp;

    fp = fopen(CONFIG_FILE_NAME, "r");
    if (fp == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        tmp = line;
        while (isspace(tmp[0])) {
            tmp++;
        }

        if (tmp[0] == '#') {
            continue;
        }

        for (;;) {
            if (get_key(&tmp, &key, &value) != 0) {
                break;
            }

            if (strcmp(key, "ID") == 0) {
                printf("%s = %s    ", key, value);
            }

            if (strcmp(key, "FORMAT") == 0) {
                printf("%s = %s    ", key, value);
            }

            if (strcmp(key, "NAME") == 0) {
                printf("%s = %s\n", key, value);
            }
        }
    }

    fclose(fp);
    return 0;
}
