
#ifndef PLAYER_LOG_H
#define PLAYER_LOG_H

#define MAX_LOG_SIZE	(20*1024)

__attribute__ ((format (printf, 2, 3)))
void log_lprint(const int level, const char *fmt, ...);

#define log_print(fmt...) 	log_lprint(0,##fmt)
#define log_error(fmt...) 	log_lprint(1,##fmt)
#define log_warning(fmt...) log_lprint(2,##fmt)
#define log_info(fmt...) 	log_lprint(3,##fmt)
/*default global_level=5,
if the level<global_level print out
*/
#define log_debug(fmt...) 	log_lprint(6,##fmt)
#define log_debug1(fmt...) 	log_lprint(7,##fmt)
#define log_debug2(fmt...) 	log_lprint(8,##fmt)
#define log_trace(fmt...) 	log_lprint(9,##fmt)

#define  DEBUG_PN() log_print("[%s:%d]\n", __FUNCTION__, __LINE__)


void log_close(void);
int log_open(const char *name);

#endif
