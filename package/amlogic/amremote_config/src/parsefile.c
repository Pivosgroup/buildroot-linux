#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_config.h"

static void str_trim( char **s )
{
     int i;
     int len=strlen(*s) ;

     for (i = len-1; i >= 0; i--)
          if ((*s)[i] <= ' ')
               (*s)[i] = 0;
          else
               break;

     while (**s)
          if (**s <= ' ')
               (*s)++;
          else
               return;
}

static int  remote_config_set(char *name,char *value,remote_config_t  *config)
{
    unsigned int i;
    unsigned int *config_para=(unsigned int*)config + 2;

    for(i=0;i<ARRAY_SIZE(config_item);i++)
        if(strcmp(config_item[i], name)==0){
            config_para[i]=strtoul(value,NULL,0);
            printf("curpara:%s  %08x\n",name, config_para[i]);
            return 0;
            }
    return -1;
}

enum {
    CONFIG_LEVEL,
    KEYMAP_LEVEL,
    MOUSEMAP_LEVEL
};
int get_config_from_file(FILE *fp, remote_config_t *remote)
{
    char  line[400];
    unsigned char parse_flag = CONFIG_LEVEL;

    while (fgets( line, 400, fp)) {
        char *name = line;
        char *comment = strchr((const char*) line,'#');
        char *value;
        unsigned short ircode, keycode;

        if (comment)
             *comment = 0;
        str_trim( &name );
	
        if (*name == '#')
            continue;

        switch(parse_flag){
            case CONFIG_LEVEL :
                if(strcasecmp(name, "key_begin")==0){
                    parse_flag = KEYMAP_LEVEL;
                    continue;
                    }
                if(strcasecmp(name, "mouse_begin")==0){
                    parse_flag = MOUSEMAP_LEVEL;
                    continue;
                    }
                value = strchr( line, '=' );
                if (value) {
                    *value++ = 0;
                    str_trim( &value );
                    }
                str_trim( &name );
                if (!*name)
                    continue;
                if(remote_config_set( name, value, remote ))
                    printf("config file has not supported parameter:%s=%s\r\n",name,value);
                continue;
            case KEYMAP_LEVEL :
                if(strcasecmp(name, "key_end")==0){
                    parse_flag = CONFIG_LEVEL;
                    continue;
                    }
                value = strchr( line, ' ' );
                if (value) {
                    *value++ = 0;
                    str_trim( &value );
                    }
                str_trim( &name );
                if (!*name)
                    continue;
                ircode = strtoul(name, NULL, 0);
                if(ircode > 0xff) continue;
                keycode = strtoul(value, NULL, 0) & 0xffff;
                if(keycode)
                    remote->key_map[ircode] = keycode;
                continue;
            case MOUSEMAP_LEVEL :
                if(strcasecmp(name, "mouse_end")==0){
                    parse_flag = CONFIG_LEVEL;
                    continue;
                    }
                value = strchr( line, ' ' );
                if (value) {
                    *value++ = 0;
                    str_trim( &value );
                    }
                str_trim( &name );
                if (!*name)
                    continue;
                ircode = strtoul(name, NULL, 0);
                if(ircode > 9) continue;
                remote->mouse_map[ircode] = strtoul(value, NULL, 0) & 0xff;
                continue;
            }
        }
    return 0;
}
