#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
Description: disable fb0 layer      
Comments:  disable osd display.          
*/
int SYS_disable_osd0(void);
/*
Description: enable fb0 layer      
Comments:  enable osd display.          
*/
int SYS_enable_osd0(void);


/*
Description:set colorkey      
Comments: refer to rgb565 as current color space.  
*/
int SYS_enable_colorkey(short key_rgb565);

/*
Description:disable colorkey      
Comments: clearall coloreky. 
*/
int SYS_disable_colorkey(void);   

int SYS_set_black_policy(int blackout);

int SYS_get_black_policy();

int SYS_set_tsync_enable(int enable);

#ifdef __cplusplus
}
#endif

#endif //_SYS_CONF_H_

