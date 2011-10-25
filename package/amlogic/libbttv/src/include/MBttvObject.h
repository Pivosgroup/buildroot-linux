#ifndef MBTTVOBJECT_H
#define MBTTVOBJECT_H

#include <QObject>

typedef struct
{
        char c_taskid[64]	 ;
        char c_name[256]	 ;
        char  c_state[64]	 ;
        char c_size[64]          ;
        char c_percent[64]       ;
        char c_rate[64]          ;

}TaskInfoOutputList;
static  TaskInfoOutputList  TaskInfoList[30];
class MBttvObject : public QObject
{
Q_OBJECT
public:
    explicit MBttvObject(QObject *parent = 0);
public:
    MBttvObject* Instance(QObject *parent = 0);
public:
    void mbInit();
    void mbUnInit();
    void mbSerialNumCheck();
    void mbRequestData();
    void mbRequestPic();
    void mbCancelRequest();
    void mbSetDownloadPath();
    void mbCurDLPathSpace();
    void RequestDataReady(unsigned int ret_value, char* err_msg, unsigned int datasrc_id, unsigned int parent_id, unsigned int rawdata, unsigned int count, unsigned int stage);
    void RequestPicReady(unsigned int ret_value, char* err_msg, unsigned int parent_id, unsigned int item_id, unsigned int image_data, unsigned int data_size, char* image_type, unsigned int stage);
    void RequestUpdataTask(unsigned int,int);
    void SendTaskListInfo();
    void LoadAllTask(char*);
    void InitTaskInfo();
    void PauseTask(unsigned int taskid );
    void CancelTask(unsigned int isCallback ,unsigned int taskid );
    void CloseTask(unsigned int taskid );
    void StartTask(unsigned int taskid );
    void SetLicence(char *);


signals:
    void requestDataGot(unsigned int ret_value, char* err_msg, unsigned int datasrc_id, unsigned int parent_id, unsigned int rawdata, unsigned int count, unsigned int stage);
    void requestPicGot(unsigned int ret_value, char* err_msg, unsigned int parent_id, unsigned int item_id, unsigned int image_data, unsigned int data_size, char* image_type, unsigned int stage);
    void updatataskinfo(unsigned int,int);
public slots:

public:
    QList<QString*> parseResult;
};

#endif // MBTTVOBJECT_H
