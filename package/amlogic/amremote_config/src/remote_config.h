#ifndef  _REMOTE_CONFIG_H
#define  _REMOTE_CONFIG_H

#include <asm/ioctl.h>

#define   REMOTE_IOC_RESET_KEY_MAPPING	    _IOW_BAD('I',3,sizeof(short))
#define   REMOTE_IOC_SET_KEY_MAPPING		    _IOW_BAD('I',4,sizeof(short))
#define   REMOTE_IOC_SET_MOUSE_MAPPING	    _IOW_BAD('I',5,sizeof(short))
#define   REMOTE_IOC_SET_REPEAT_DELAY		    _IOW_BAD('I',6,sizeof(short))
#define   REMOTE_IOC_SET_REPEAT_PERIOD	    _IOW_BAD('I',7,sizeof(short))

#define   REMOTE_IOC_SET_REPEAT_ENABLE		_IOW_BAD('I',8,sizeof(short))
#define	REMOTE_IOC_SET_DEBUG_ENABLE			_IOW_BAD('I',9,sizeof(short)) 
#define	REMOTE_IOC_SET_MODE					_IOW_BAD('I',10,sizeof(short)) 

#define   REMOTE_IOC_SET_RELEASE_DELAY		_IOW_BAD('I',99,sizeof(short))
#define   REMOTE_IOC_SET_CUSTOMCODE   			_IOW_BAD('I',100,sizeof(short))
//reg
#define   REMOTE_IOC_SET_REG_BASE_GEN			_IOW_BAD('I',101,sizeof(short))
#define   REMOTE_IOC_SET_REG_CONTROL			_IOW_BAD('I',102,sizeof(short))
#define   REMOTE_IOC_SET_REG_LEADER_ACT 		_IOW_BAD('I',103,sizeof(short))
#define   REMOTE_IOC_SET_REG_LEADER_IDLE 		_IOW_BAD('I',104,sizeof(short))
#define   REMOTE_IOC_SET_REG_REPEAT_LEADER 	_IOW_BAD('I',105,sizeof(short))
#define   REMOTE_IOC_SET_REG_BIT0_TIME		 _IOW_BAD('I',106,sizeof(short))

//sw
#define   REMOTE_IOC_SET_BIT_COUNT			 	_IOW_BAD('I',107,sizeof(short))
#define   REMOTE_IOC_SET_TW_LEADER_ACT		_IOW_BAD('I',108,sizeof(short))
#define   REMOTE_IOC_SET_TW_BIT0_TIME			_IOW_BAD('I',109,sizeof(short))
#define   REMOTE_IOC_SET_TW_BIT1_TIME			_IOW_BAD('I',110,sizeof(short))
#define   REMOTE_IOC_SET_TW_REPEATE_LEADER	_IOW_BAD('I',111,sizeof(short))

#define   REMOTE_IOC_GET_TW_LEADER_ACT		_IOR_BAD('I',112,sizeof(short))
#define   REMOTE_IOC_GET_TW_BIT0_TIME			_IOR_BAD('I',113,sizeof(short))
#define   REMOTE_IOC_GET_TW_BIT1_TIME			_IOR_BAD('I',114,sizeof(short))
#define   REMOTE_IOC_GET_TW_REPEATE_LEADER	_IOR_BAD('I',115,sizeof(short))


#define   REMOTE_IOC_GET_REG_BASE_GEN			_IOR_BAD('I',121,sizeof(short))
#define   REMOTE_IOC_GET_REG_CONTROL			_IOR_BAD('I',122,sizeof(short))
#define   REMOTE_IOC_GET_REG_LEADER_ACT 		_IOR_BAD('I',123,sizeof(short))
#define   REMOTE_IOC_GET_REG_LEADER_IDLE 		_IOR_BAD('I',124,sizeof(short))
#define   REMOTE_IOC_GET_REG_REPEAT_LEADER 	_IOR_BAD('I',125,sizeof(short))
#define   REMOTE_IOC_GET_REG_BIT0_TIME		 	_IOR_BAD('I',126,sizeof(short))
#define   REMOTE_IOC_GET_REG_FRAME_DATA		_IOR_BAD('I',127,sizeof(short))
#define   REMOTE_IOC_GET_REG_FRAME_STATUS	_IOR_BAD('I',128,sizeof(short))


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef   struct{
       unsigned short *key_map;
       unsigned short *mouse_map;
       unsigned int repeat_delay;
       unsigned int repeat_peroid;
       unsigned int work_mode ;
	unsigned int repeat_enable;
	unsigned int factory_code;
	unsigned int release_delay;
	unsigned int debug_enable;
//sw
	unsigned int 	bit_count;
	unsigned int 	tw_leader_act;
	unsigned int 	tw_bit0;
	unsigned int   tw_bit1;
	unsigned int 	tw_repeat_leader;
//reg
	unsigned int  reg_base_gen;
	unsigned int  reg_control;
	unsigned int  reg_leader_act;
	unsigned int  reg_leader_idle;
	unsigned int  reg_repeat_leader;
	unsigned int  reg_bit0_time;
}remote_config_t;

//these string must in this order and sync with struct remote_config_t
static char*  config_item[18]={
    "repeat_delay",
    "repeat_peroid",
    "work_mode",
    "repeat_enable",
    "factory_code",
    "release_delay",
    "debug_enable",
//sw
    "bit_count",
    "tw_leader_act",
    "tw_bit0",
    "tw_bit1",
    "tw_repeat_leader",
//reg
    "reg_base_gen",
    "reg_control",
    "reg_leader_act",
    "reg_leader_idle",
    "reg_repeat_leader",
    "reg_bit0_time"
};

static int remote_ioc_table[18]={
    REMOTE_IOC_SET_REPEAT_DELAY,
    REMOTE_IOC_SET_REPEAT_PERIOD,
    REMOTE_IOC_SET_MODE,
    REMOTE_IOC_SET_REPEAT_ENABLE,
    REMOTE_IOC_SET_CUSTOMCODE,
    REMOTE_IOC_SET_RELEASE_DELAY,
    REMOTE_IOC_SET_DEBUG_ENABLE,
//sw
    REMOTE_IOC_SET_BIT_COUNT,
    REMOTE_IOC_SET_TW_LEADER_ACT,
    REMOTE_IOC_SET_TW_BIT0_TIME,
    REMOTE_IOC_SET_TW_BIT1_TIME,
    REMOTE_IOC_SET_TW_REPEATE_LEADER,
//reg
    REMOTE_IOC_SET_REG_BASE_GEN,
    REMOTE_IOC_SET_REG_CONTROL	,
    REMOTE_IOC_SET_REG_LEADER_ACT,
    REMOTE_IOC_SET_REG_LEADER_IDLE,
    REMOTE_IOC_SET_REG_REPEAT_LEADER,
    REMOTE_IOC_SET_REG_BIT0_TIME
};

extern int set_config(remote_config_t *remote, int device_fd);
 extern int get_config_from_file(FILE *fp, remote_config_t *remote);

#endif
