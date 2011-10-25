

/**
 * update call back function
 * @param taskid                    [out]   the call back result or error number
 */
typedef void (*update_callback)(int succ);

#define G_IMG_PATH "/mnt/C/download_img"

#define G_NETMOVIE_IMG_PATH "/mnt/C/app_NIKE.img"

#define G_IMG_EXISTED_PATH "/mnt/C/is_new_img"

#define G_VOB_EXISTED_PATH "/mnt/C/is_new_vob"

/**
 * reset update state machine
 * @return                                  error id
 */
int ResetUpdateManager();


/**
 * add a task
 * @param xml_url             	[in]    the url of the xml file
 * @param c_words_ver			[in]    the words version
 * @param xml_save_path                [in]    the path to save the xml file
 * @param img_save_path	                    [in]   	the save path of the img
 * @return                                  error id
 */
int StartUpdateManager(char* xml_url,char * c_words_ver, char* xml_save_path,char* img_save_path);


/**
 * get the current status of update state machine
 * @return                                  state id
 */
int GetUpdateStatus();


/**
 * initialize the update state machine
 * @return                                  error id
 */
int InitUpdateManager();


/**
 * stop the update state machine
 * @return                                  error id
 */
int StopUpdateManager();


/**
 * check the checksum of the img
 * @param source_path              	[in]    the saved path of img
 * @return                                  error id
 */
int CheckImg(char* img_path);


/**
 * start a task to burn img
 * @param source_path              	[in]    the callback function
 * @return                                  error id
 */
int BurnImgEx(update_callback update_cb);