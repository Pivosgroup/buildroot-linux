#ifndef REF_UPDATE_MANGER_H
#define REF_UPDATE_MANGER_H

/**
 * update call back function
 * @param taskid                    [out]   the call back result or error number
 */
typedef void (*update_callback)(int succ, char * net_version);

enum ref_update_error_no
{
	no_error = 0,
	error_not_idle = -1,
	error_thread_create = -2,
	error_load_xml = -3,
	error_get_node = -4,
	error_get_node_name = -5,
	error_no_child = -6,
	error_get_node_value = -7,
	error_get_current_version = -8,
	error_compare_version = -9,
	error_version_to_int = -10,
	error_current_version_is_newest = -11,
	error_no_disk= -12,
	error_not_enough_space = -13,
	error_download_img = -14,
	error_download_xml = -15,
};

enum ref_update_status
{
	status_idle = 0,
	status_download_xml,
	status_parse_xml,
	status_comp_version,
	status_wait_for_down_img_command,
	status_download_img,
	status_wait_for_upgrade_command,
};
/**
 * reset update state machine
 * @return                                  error id
 */
int ResetRefUpdateManager();


/**
 * add a task
 * @param xml_url             	[in]    the url of the xml file
 * @param c_words_ver			[in]    the words version
 * @param callback                          [in]    the callback for the controls of high layer
 * @param xml_save_path                [in]    the path to save the xml file
 * @param img_save_path	                    [in]   	the save path of the img
 * @return                                  error id
 */
int StartRefUpdateManager(char* xml_url, char * c_words_ver, update_callback callback, char* xml_save_path,char* img_save_path);


/**
 * get the current status of update state machine
 * @return                                  state id
 */
int GetRefUpdateStatus();

/**
 * get the current status of error
 * @return                                  state id
 */
int GetRefUpdateErrorStatus();

/**
 * Reset the update state machiine
 * @return                                  nothing
 */
void RefResetUpdateManager();

/**
 * start to download IMG
 * @return                                  state id
 */
int RefUpdateStartDownIMG();

/**
 * check if the upgrade IMG is dowloaded
 * @return                                  state id
 */
int RefUpdateCheckIMGIsReady();

/**
 * initialize the update state machine
 * @return                                  error id
 */
int InitRefUpdateManager();


/**
 * stop the update state machine
 * @return                                  error id
 */
int StopRefUpdateManager();

/**
* set the update URL for update system
* waring: this url is only the default url, it can be changed by UI control!!!
*/
int SetRefUpdateURL(char * url);

/**
* cancel the img downloading task
*/
void CancelImgDown(void);
#endif
