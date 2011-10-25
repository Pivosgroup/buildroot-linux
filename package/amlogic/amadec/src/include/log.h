#ifndef LOG_H
#define LOG_H

//#define ENABLE_SYSLOG 0

#if ENABLE_SYSLOG
#define lp(x...) syslog(x)
#else
#define lp(x...) log_print(x)
#endif

#ifdef  __cplusplus
extern "C" {
#endif

extern int log_open(const char *name);
extern void log_close(void);
extern void log_print(const int level, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

#ifdef  __cplusplus
}
#endif

#endif /* LOG_H */
