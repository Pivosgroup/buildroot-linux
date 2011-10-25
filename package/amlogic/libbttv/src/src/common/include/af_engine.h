#ifndef _AFRAME_ENGINE_H
#define _AFRAME_ENGINE_H
#include "includes.h"
//#include "ioapi.h"
//#include "listop.h"
//#include "af_osd.h"

//#define AF_TEST_CORE

//#define RES_ROOT "/res/"
//#define RES_DIR "/res/resource/"
extern  char * RES_DIR;
extern  char * RES_RAM_DIR;

#define NAME_LEN 16

#ifndef NULL
#define NULL 0
#endif

#define TRUE 1
#define FALSE 0

struct control_t_;

#define AVMem_malloc(len) (char*)malloc(len)
#define AVMem_free(p)   do{ if(p){free(p);p=0;}}while(0)
#define AVMem_calloc calloc
#define AVMem_realloc realloc

typedef unsigned cond_item_t;

typedef unsigned retval_t;

typedef int (*method_func)(struct control_t_* cntl, cond_item_t* param);                

typedef int (*aframe_shell_print_t)(char * buff);

typedef struct {
    char name[NAME_LEN];
}name_t;

typedef unsigned int handle_t; // 31-22 for page, 21-16 for wnd, 15-10 for ctnl, 9-0 for prop/method
        
//SYSTEM_HANDLE is used as src_handle when system call af_create_message(...)
#define SYSTEM_HANDLE           0
#define SYSTEM_DEFAULT_PAGE     1

#define MAX_RULE_DEPTH  32

#define RULE_TASK_DONE      0
#define RULE_TASK_BREAK     1
#define RULE_TASK_PENDING   2
#define RULE_TASK_ERROR     -1

#define PAGE_BIT_SHIFT  22
#define WND_BIT_SHIFT   19
#define CNTL_BIT_SHIFT   10

#define ALL_PAGE            ((1<<(32-PAGE_BIT_SHIFT))-1)
#define ALL_WND             ((1<<(PAGE_BIT_SHIFT-WND_BIT_SHIFT))-1)
#define ALL_CNTL            ((1<<(WND_BIT_SHIFT-CNTL_BIT_SHIFT))-1)
#define ALL_PROP            ((1<<CNTL_BIT_SHIFT)-1)
#define ARRAY_ITEM_FLAG     ((1<<(CNTL_BIT_SHIFT-1)))


#define DISPLAY_PAGE        (ALL_PAGE-1)
#define FOCUS_PAGE          (ALL_PAGE-2)
#define DYNAMICAL_CNTL_PAGE  (ALL_PAGE-3)

#define GET_PAGE(handle)    (((handle)>>PAGE_BIT_SHIFT)&ALL_PAGE)
#define GET_WND(handle)     (((handle)>>WND_BIT_SHIFT)&ALL_WND)
#define GET_CNTL(handle)    (((handle)>>CNTL_BIT_SHIFT)&ALL_CNTL)
#define GET_PROP(handle)    ((handle)&ALL_PROP)
#define GET_ARRAY_PROP(handle)  ((handle)&(~(1<<(CNTL_BIT_SHIFT-1))))
#define GET_METHOD(handle)  GET_PROP(handle)

#define BROADCAST_PAGE      (ALL_PAGE<<PAGE_BIT_SHIFT)
#define BROADCAST_WND       (ALL_WND<<WND_BIT_SHIFT)
#define BROADCAST_CNTL      (ALL_CNTL<<CNTL_BIT_SHIFT)
#define BROADCASE_PROP      (ALL_PROP)
#define BROADCAST_ALL       (BROADCAST_PAGE|BROADCAST_WND|BROADCAST_CNTL|BROADCASE_PROP)
#define BROADCAST_DISPLAY_PAGE ((DISPLAY_PAGE<<PAGE_BIT_SHIFT)|BROADCAST_WND|BROADCAST_CNTL|BROADCASE_PROP)
#define BROADCAST_FOCUS_PAGE ((FOCUS_PAGE<<PAGE_BIT_SHIFT)|BROADCAST_WND|BROADCAST_CNTL|BROADCASE_PROP)

#define GET_PAGE_HANDLE(handle) ((handle&BROADCAST_PAGE)|BROADCAST_WND|BROADCAST_CNTL|BROADCASE_PROP)
#define GET_WND_HANDLE(handle) ((handle&(BROADCAST_PAGE|BROADCAST_WND))|BROADCAST_CNTL|BROADCASE_PROP)
#define GET_CNTL_HANDLE(handle) ((handle&(BROADCAST_PAGE|BROADCAST_WND|BROADCAST_CNTL))|BROADCASE_PROP)

#define GET_PROP_TYPE(ctrl, prop_id)        ((ctrl)->cntl_info->prop[prop_id].type)
#define GET_PROP_NAME(ctrl, prop_id)        ((ctrl)->cntl_info->prop[prop_id].name)
#define GET_PROP_VALUE(ctrl, prop_id)       ((ctrl)->value[prop_id])

#define isArrayItem(handle) ((handle&ARRAY_ITEM_FLAG)&&(GET_PROP(handle)!=ALL_PROP))

#define RES_TYPE_ICON 1
#define RES_TYPE_FILE 2
#define RES_TYPE_FONT 3

#define GET_RES_TYPE(id) ((id)>>16)

typedef unsigned prop_method_t; // 31-22 for page, 21-16 for wnd, 15-10 for ctnl, 9-0 for prop/method

typedef enum {
    CONST_INT = 0,
    CONST_STR,
    RESOURCE_ID,
    PROP,
    METHOD,
    MSG_PARAM,
    HANDLE,
    MSG_SENDER,
} cond_item_type_t;     // types in condition and property, method param and method return value

typedef enum {
    AND,
    OR
} cond_flag_t;

typedef enum {
    FLAG_COND_START = 0,
    FLAG_COND_ELSE,
    FLAG_COND_END
} ifelse_flag_t;

typedef enum {
    NA = 0,
    EQ,
    NE,
    GT,
    LT,
    GE,
    LE,
    LAST_COND,
    AND_LAST_COND = 8
} cond_type_t;          // condition, NA for no condition

typedef enum {
    DO_NOTHING = 0,
    CALL_METHOD,
    SET_PROPERTY,
    SEND_MESSAGE,
    TRANSFER_MESSAGE,
    KILL_MESSAGE,
    IF_ELSE_CONDITION,
    DEFAULT_MSG_PROCESS,
    DISABLE_MSG_PROCESS,
    RULE_BREAK,
    ASYNC_METHOD, 
    START_WHILE, 
} action_type_t;        // action following condition

typedef unsigned action_t;             // there are 9 act type -- see action_type_t
#define state_type_t action_type_t

typedef cond_item_t action_param_t;

typedef struct {
    cond_flag_t cond_flag;
    cond_type_t cond_type;          // condition type -- NA=0 for none
    cond_item_type_t left_type;
    cond_item_t left;
    int left_index;
    cond_item_type_t right_type;
    cond_item_t right;
    int right_index;
} cond_t;

typedef struct {
    unsigned num_cond;
    cond_t* cond;
    unsigned num_true_state;
    unsigned false_state_slot;
    unsigned num_false_state;
} ifelse_state_t;

typedef struct {
    action_t action;
    int action_index;
    unsigned num_param;
    retval_t ret_prop;
    int ret_prop_index;
    cond_item_type_t *action_param_type;
    action_param_t *action_param;   // if call method, it contains the parameters; 
                                    // when set property it contains item to assign;
    int *action_param_index;
} common_state_t;

typedef struct {
    action_t action;
    unsigned num_action_in_loop;
    unsigned num_state_in_loop;
} while_state_t;

typedef struct {
    state_type_t type;
    union{
        ifelse_state_t ifelse;
        common_state_t common;
        while_state_t whiledo;
    } content;
} state_t;

typedef struct {
    unsigned msg_id;                // message in
    unsigned number_state;
    unsigned total_state;
    unsigned cur_slot;
    state_t *state;
} rule_t;

#define METHOD_CALL_FLAG_BLOCKING 1
#define METHOD_FLAG_BLOCKABLE 1
typedef struct {
    char name[NAME_LEN];            // method name
    unsigned char flag;
    unsigned num_param;             // number of parameter
    cond_item_type_t* param_type;   // param types
    name_t* param_name;             // param names
    cond_item_type_t output_type;
    int (*func)(struct control_t_* cntl, cond_item_t* param);               // 
} method_t;

#define ARRAY_FLAG                      8
#define ARRAY_STRU_SIZE_SHIFT_COUNT     4

typedef struct {
    char name[NAME_LEN];        // property name
    cond_item_type_t type;      // property type
    INT32U array_type;          //property type
#if 1
//need to be removed after UI tool give enum initial value beyond -1
    char enum_flag;
#endif    
    //cond_item_t init_value;     //
} prop_t;

#define METHOD_INIT_POS             0
#define METHOD_UNINIT_POS           1
#define METHOD_MSGPROCESS_POS       2
#define METHOD_SERIALNUM_CHECK_POS  3
#define DEFAULT_METHOD_COUNT        4

#define CONTROL_FLAG_HAS_INPUT_PROC 1
typedef struct {
    char name[NAME_LEN];        // control name
    unsigned char num_prop;     // number of property
    unsigned char num_method;   // number of method
    unsigned char num_acpt_cntl_type; // number of acceptable control type
    unsigned type;              // control type
    
    method_t Init;
    method_t UnInit;
    method_t MsgProcess;
    method_t SerialNumCheck;
    
    method_t* method;           // array contains num_method of method -- method[num_method]
    prop_t* prop;               // array contains num_prop of property -- prop[num_prop]   
    unsigned * acpt_cntl_types; // array contains  acceptable control type -- acpt_cntl_types[num_acpt_cntl_type]
    cond_item_type_t* param_type_pool;
    unsigned char flag;
} cntl_info_t;

#define CNTL_FLAG_INITED       0x80
#define CNTL_FLAG_HIDDEN       0x40
#define CNTL_FLAG_FOCUS        0x20
#define CNTL_FLAG_STATE_SHOW   0x01
#define CNTL_FLAG_DISABLE_DRAW 0x02
#define CNTL_FLAG_RULE_CALL    0x04

typedef struct control_t_{
    char *name;
    char *type;                 // control type -- to find the cntl_info
    handle_t parent_handle;     // parent page/window
    handle_t owner_handle;      // owner_handle's some prop is the handle of this control 
    unsigned char flag;         // control flag
    unsigned char id;           // control id
    unsigned short num_value;    // number of property values
    unsigned short num_rule;     // number of rules
    unsigned char block_draw_msg; //block MSG_TYPE_DRAW/MSG_TYPE_HIDE 
    cond_item_t *value;         // array of property value 
    rule_t* rules;              // array of rules
    cntl_info_t *cntl_info;     // pointer to cntl_info
    void *private_data;         // pointer to private data used by control instance
    unsigned short init_sequence;
    cond_item_type_t *value_type;
    unsigned asyn_msg_id;
    handle_t asyn_msg_target_handle;
} control_t;


#endif
