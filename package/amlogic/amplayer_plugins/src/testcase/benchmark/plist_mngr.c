#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "list.h"

#include "taskmngr.h"
#include "mp_log.h"
#include "mp_api.h"

#include "plist_mngr.h"

typedef enum{
    MP_INVALID =100,
    MP_FLIST_IN ,
    MP_DECODED_IN,
    MP_PLIST_IN,
}MP_itemwhereIn;


typedef struct _MP_mediaItem_{
    char *url;
    int index;
    MP_itemwhereIn where_in;
    struct list_head flist;//play list,just one node 
}MP_mediaItem;


static struct list_head gFileList;
static int gTotalItemsInFList = 0;
static int gFileListInitFlag = 0;


static pthread_mutex_t mp_filelist_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MP_FLIST_LOCK()   pthread_mutex_lock(&mp_filelist_mutex)
#define MP_FLIST_UNLOCK() pthread_mutex_unlock(&mp_filelist_mutex)

//play list

int MP_AddFileToList(const char* url)
{
    
    MP_mediaItem* item = NULL;
    if(NULL == url && strlen(url) >FILE_URL_MAX&&strlen(url) ==0)
    {
        printf("url must not null and less than 1024\n");
        return -1;
    }  
    
    MP_FLIST_LOCK();
    if(gFileListInitFlag ==0)
    {
        INIT_LIST_HEAD(&gFileList);
        gFileListInitFlag = 1;
    }
    
    if(gTotalItemsInFList >=PLIST_FILE_MAX)
    {
        log_err("overflow max file counts:%d\n",PLIST_FILE_MAX);
        return -1;

    }
    
    item = (MP_mediaItem*)malloc(sizeof(MP_mediaItem));
    if(NULL ==item)
    {
        log_err("failed to malloc memory:%s\n",__FUNCTION__);
        return -1;
    }    
    
    item->url= strndup(url,FILE_URL_MAX);
    
    INIT_LIST_HEAD(&item->flist);
    item->where_in = MP_FLIST_IN;
    gTotalItemsInFList++;
    item->index = gTotalItemsInFList;
    
    list_add(&item->flist, &gFileList);
    
    
    
    MP_FLIST_UNLOCK();

    return 0;    
    
}

int MP_RemoveFileFromListByUrl(const char* url);
int MP_RemoveFileFromListByIndex();
int MP_DumpPlayList()
{
    MP_mediaItem *pos = NULL;
    MP_mediaItem *tmp = NULL;
    printf("*******************playlist files dump start*****************************\n");
    MP_FLIST_LOCK();
    if(!list_empty_careful(&gFileList)){
        list_for_each_entry_safe_reverse(pos, tmp, &gFileList, flist)
        {
           printf("%d'st file url:%s\n",pos->index,pos->url);
        }
    }
    MP_FLIST_UNLOCK();
    printf("*******************playlist files dump end******************************\n");
    return 0;
    
}
int MP_ClearAllPlayList()
{
    MP_mediaItem *pos = NULL;
    MP_mediaItem *tmp = NULL;
    printf("*******************Clean All items from list start*****************************\n");
    MP_FLIST_LOCK();
    list_for_each_entry_safe(pos, tmp, &gFileList, flist)
    {
        list_del(&pos->flist);
        free(pos->url);
        free(pos);
        gTotalItemsInFList--;
    }
    MP_FLIST_UNLOCK();
    printf("*******************Clean All items from list end*****************************\n");
    return 0;
}
    
//=========================================
int MP_AddItemToList(MP_mediaItem *item)
{
    if(item!=NULL)
    {
        MP_FLIST_LOCK();
        if(gFileListInitFlag ==0)
        {
            INIT_LIST_HEAD(&gFileList);
            gFileListInitFlag = 1;
        }

        if(gTotalItemsInFList > PLIST_FILE_MAX)
        {
            log_err("overflow max file counts:%d\n",PLIST_FILE_MAX);
            return -1;

        }

        item->index = gTotalItemsInFList;
        list_add(&item->flist, &gFileList);

        gTotalItemsInFList++;
        
        MP_FLIST_UNLOCK();

        return 0;
        
    }
    
    return -1;
}
MP_mediaItem* MP_GetItemFromList(MP_AutoPLayMode mode)
{
    MP_mediaItem* item = NULL;
    MP_FLIST_LOCK();

    if(gTotalItemsInFList == 0)
    {
        MP_FLIST_UNLOCK();
        return NULL;    
    }
    
    switch(mode)
    {
        case MP_PLAY_ORDER:
           
            if (!list_empty(&gFileList))
            {
                item = list_entry(gFileList.prev, MP_mediaItem, flist);
                list_del(&item->flist);
                gTotalItemsInFList --;    
            }
            break;
        case MP_PLAY_LOOP:
            if (!list_empty(&gFileList))
            {
                MP_mediaItem *item_new = NULL;
                item_new = (MP_mediaItem*)malloc(sizeof(MP_mediaItem));
                if(item_new == NULL)
                {
                    log_err("failed to malloc for item,%s\n",__FUNCTION__);
                    MP_FLIST_UNLOCK();
                    return  NULL;
                }
                item = list_entry(gFileList.prev, MP_mediaItem, flist);                 
                list_del(&item->flist);
                item_new->url = strdup(item->url);
                INIT_LIST_HEAD(&item_new->flist);
                item_new->index = item->index;
                item_new->where_in = item->where_in;
                list_move(&item_new->flist,&gFileList);
                
            }
            break;
        case MP_PLAY_SHUFFLE:
            break;
        case  MP_PLAY_BOTH:
            break;
        default:
            break;
    }
    
    MP_FLIST_UNLOCK();

    return item;    
    
}





//========================================

static int gActiveMediaid = -1; //if -1,means inactive.
static int gParseredMediaid = -1;//if -1,means inactive
static MP_AutoPLayMode g_mode = MP_PLAY_ORDER;
//static pthread_mutex_t mp_playlist_mutex = PTHREAD_MUTEX_INITIALIZER;

//#define MP_PLIST_LOCK()   pthread_mutex_lock(&mp_playlist_mutex)
//#define MP_PLIST_UNLOCK() pthread_mutex_unlock(&mp_playlist_mutex)

static int task_id =-1;

static void* MP_AutoPlayTask(void* args);

int MP_StartAutoPlayTask(MP_AutoPLayMode mode)
{
    if(task_id >0)
    {
        log_err("just run a autoplay task,drop this command\n");
        return -1;
    }
    g_mode = mode;
    MP_taskBlockSignalPE();    
    MP_taskSetCancel();   
   
    task_id = MP_taskCreate("MP_AutoPlayTask",1, 409600,MP_AutoPlayTask,NULL);
    if(task_id == -1)
    {        
        log_err("Failed to create autoplay task thread !\n");
        return -1;
    }
    return 0;    
}
int MP_PauseAutoPlayTask()
{
    if(task_id >0 )
    {
        return 0;
    }
    return -1;
}


int MP_ResumeAutoPlayTask()
{

    if(task_id >0 )
    {
        return 0;
    }
    return -1;
}
int MP_SetAutoPlayMode(MP_AutoPLayMode mode)
{
    g_mode= mode;
    return 0;
}
int MP_StopAutoPlayTask()
{
    if(task_id>0)
    {
        MP_taskDelete(task_id);
        task_id = -1;
        MP_ClearAllPlayList();
        if(gActiveMediaid>0)
            MP_stop(gActiveMediaid);
        if(gParseredMediaid >0)
            MP_stop(gParseredMediaid); 

        gActiveMediaid = -1;
        gParseredMediaid = -1;
        return 0;
    }
    
    return -1;

}

int MP_InactiveMediaID(int mediaID)
{
    //MP_PLIST_LOCK();
    
    if(mediaID == gActiveMediaid)
    {
        log_info("inacitve media id for playing,media id is%d\n",mediaID);
        gActiveMediaid = -1;

    }
    if(mediaID ==gParseredMediaid)
    {
        log_info("inacitve media id for parsing,media id is%d\n",mediaID);      
        if(gParseredMediaid>0)
            MP_stop(gParseredMediaid);
        gParseredMediaid = -1;

    }
    //MP_PLIST_UNLOCK();    
    return 0;
}

int MP_ActiveMediaID(int mediaID)
{
    //MP_PLIST_LOCK();    

    if(mediaID <0)
        return -1;
    if(gActiveMediaid == mediaID)
    {
        log_info("acitve media id for playing,media id is%d\n",mediaID);
        gActiveMediaid = mediaID;

    }
    else if(gParseredMediaid == mediaID)
    {
        log_info("acitve media id for parsing,media id is%d\n",mediaID);
        MP_GetMediaInfo(NULL,mediaID);
        gParseredMediaid = mediaID;
    }
    //MP_PLIST_UNLOCK();    
    return 0;
    
}
int MP_PlayNextFile()
{   
    MP_FLIST_LOCK();
    if(gTotalItemsInFList >1)
    {
        if(gActiveMediaid >0)
        {
            MP_stop(gActiveMediaid);
            MP_CloseMediaID(gActiveMediaid);
            gActiveMediaid = -1;

        }
        if(gParseredMediaid> 0)
        {
            MP_stop(gParseredMediaid);
            MP_CloseMediaID(gParseredMediaid);
            gParseredMediaid = -1;
        }
    }
    MP_FLIST_UNLOCK();
    return 0;
    
}
//=========================================

static void* MP_AutoPlayTask(void* args)
{     
    MP_mediaItem *item = NULL;    
    
    while(1)
    {
        //MP_PLIST_LOCK();           
        
        if(gActiveMediaid <0&&gParseredMediaid<0)
        {
            
            item = MP_GetItemFromList(g_mode);   
            if((NULL !=item)&&(NULL !=item->url))
            {
                log_con("autoplay next item,item index:%d\n",item->index);
                gActiveMediaid = MP_PlayMediaSource(item->url,0,0);  
                log_info("Get play task id:%d\n",gActiveMediaid);
                if(gParseredMediaid >0)
                {
                    MP_stop(gParseredMediaid);
                    MP_CloseMediaID(gParseredMediaid);
                    gParseredMediaid = -1;
                }
                gParseredMediaid = MP_AddMediaSource(item->url);
                log_info("Get parser task id:%d\n",gParseredMediaid);
                free(item->url);
                free(item);
                //MP_PLIST_UNLOCK();
                usleep(100000);//100ms
            }

        }
        else
        {
            usleep(100000);//100ms
            //MP_PLIST_UNLOCK();

        }
    };
    return NULL;
}

//==============================================
// about scan dir
#include <dirent.h> 
#include <sys/types.h> 
#include <sys/stat.h>


#define DEFAULT_DIR_DEPTH 2
#define DIR_PATH_MAX 512

static int _filenameFilter(const char *url)
{
    
    if(NULL != url)
    {
        if(NULL != strstr(url,".rmvb") ||NULL !=strstr(url,".rm")\
            ||NULL !=strstr(url,".avi")||NULL!=strstr(url,".mp3")\
            ||NULL !=strstr(url,".mpeg")||NULL!=strstr(url,".mpg")\
            ||NULL !=strstr(url,".ts")||NULL!=strstr(url,".mov")\
            ||NULL !=strstr(url,".mkv")||NULL!=strstr(url,".mp4")\
            ||NULL !=strstr(url,".aac")||NULL!=strstr(url,".ogg")\
            ||NULL !=strstr(url,".MP3")||NULL!=strstr(url,".m4a")\
            ||NULL !=strstr(url,".AVI")||NULL!=strstr(url,".vc1")\
            ||NULL !=strstr(url,".m2ts")||NULL!=strstr(url,".3gp")\
            ||NULL !=strstr(url,".MOV")||NULL!=strstr(url,".VOB")\
            ||NULL !=strstr(url,".flac")||NULL!=strstr(url,".mp2")\
            ||NULL !=strstr(url,".m4a")||NULL!=strstr(url,".ac3")\
            
            )
        {
            log_con("Filter complete,just a mutlimedia file\n");
            return 0;
        }
        else
        {
             log_info("Not a match file,just omit it\n");
             return -1;
        }
    }
    
    return -1;
}

static void _scanDirAndToPlayList( char *path, int indent )
{
    struct dirent* ent = NULL;
    DIR *pDir;
    char dirname[DIR_PATH_MAX];
    struct stat statbuf; 

    memset(dirname,0,DIR_PATH_MAX*sizeof(char));    
    //open dir
    pDir=opendir(path);
    if(pDir==NULL)
    {
        log_err("Cannot open directory:%s\n", path );
        return;
    }
    chdir(path);    
    while( (ent=readdir(pDir) )!=NULL )
    {
        //get absolute path
        snprintf(dirname,DIR_PATH_MAX,"%s/%s", path,ent->d_name );    
        //get file info
         if(lstat(dirname,&statbuf) < 0)
        { 
            log_err("lstat error\n"); 
            break; 
        }        
        
         //judge if a directory or file
        if( S_ISDIR(statbuf.st_mode) )
        {
            //omit current directory or uplevel directory
            if(strcmp( ".",ent->d_name) == 0 || strcmp( "..",ent->d_name) == 0)
            {
                continue;
            }    
            //
            log_info( "%*s sub-directory:%s/\n", indent, "", ent->d_name );
            //iteration
            _scanDirAndToPlayList(dirname,indent+4);                
        }
        else
        {
            int ret = -1;
            log_info("%*s file:%s [fullpath:%s]\n", indent, "", ent->d_name,dirname);

            //do filter
            ret = _filenameFilter(ent->d_name);
            if(ret == 0)
            {
                ret = MP_AddFileToList(dirname);   
            }
        }        
    }
    closedir(pDir);
}

static void* MP_ScanDirAndToPlayListTask(void* args)
{
    char* spath = (char*)args;

    log_info("%s:Add dir task,path:%s\n",__FUNCTION__,spath);
    
    _scanDirAndToPlayList(spath,DEFAULT_DIR_DEPTH);     
    return NULL;
    
}
int MP_AddDirToPlayList(char* path)
{
    struct stat s; 
    int task_scan;
    if(NULL ==path)
    {
        log_err("invalid directory,please check it\n");
        return -1;        
    }
    if(lstat(path,&s) < 0)
    { 
        log_err("lstat error\n"); 
        return -2; 
    } 
    if(!S_ISDIR(s.st_mode))
    {
        log_err("%s is not a direction name\n", path); 
        return -3;        
    }

    log_info("add directory pathname:%s\n",path);

    MP_taskBlockSignalPE();    
    MP_taskSetCancel();   
   
    task_scan= MP_taskCreate("MP_ScanDirAndToPlayListTask",1, 409600,MP_ScanDirAndToPlayListTask,(void*)path);
    if(task_scan == -1)
    {        
        log_err("Failed to create scan directory task thread !\n");
        return -1;
    }
    return 0;
}


