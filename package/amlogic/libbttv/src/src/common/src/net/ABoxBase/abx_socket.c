#include "abx_socket.h"
#include "abx_thread.h"
#include "abx_mem.h"


typedef struct {
    void * arg;
    char * name;
    abx_resolver_cb_t callback;
} abx_res_param_t ;


#ifdef AVOS

#include <net/resolver/resolver.h>
#include <string.h>



static void abx_resolver_callback (void *arg, int status, struct hostent *host_result, void *res)
{
    abx_res_param_t * param = (abx_res_param_t *)arg;
    if (status != RES_SUCCESS || host_result == NULL)
    {
        param->callback(param->name, param->arg, NULL);
    }
    else
    {
        param->callback(param->name, param->arg, host_result);
    }
     if (host_result)
	AVMem_free(host_result);
    res_free(res);
    abx_free(param->name);
    abx_free(param);
}


abx_errid_t abx_gethostbyname_a(const char * name, void * arg, abx_resolver_cb_t callback)
{
    RESOLVER * res;
    char * buf = (char *)abx_malloc(strlen(name) + 1);
    if (!buf)
        return ABXERR_OUTOFMEMORY;
    abx_res_param_t * param = (abx_res_param_t *)abx_malloc(sizeof(abx_res_param_t));
    if (!param) {
        abx_free(buf);
        return ABXERR_OUTOFMEMORY;
    }

    param->name = buf;
    param->arg = arg;
    strcpy(buf, name);
    param->callback = callback;

    res = gethostbyname_a(param->name, AF_INET, abx_resolver_callback, param);
    if (res == NULL)
        return ABXERR_DNS_ERR;

    return ABXERR_OK;
}


#else

int ABX_STDCALL abx_asyn_res_thread_win(void * data)
{
    abx_res_param_t * param = data;
    struct hostent * host_result = gethostbyname(param->name);
    param->callback(param->name, param->arg, host_result);
    abx_free(param->name);
    abx_free(param);
    return 0;
}



abx_errid_t abx_gethostbyname_a(const char * name, void * arg, abx_resolver_cb_t callback)
{
    abx_res_param_t * param = (abx_res_param_t *)abx_malloc(sizeof(abx_res_param_t));
    param->name = (char *)abx_malloc(strlen(name) + 1);
    param->arg = arg;
    strcpy(param->name, name);
    param->callback = callback;
    abx_thread_new(abx_asyn_res_thread_win, param, 0,0,0);
    return ABXERR_OK;
}

#endif

