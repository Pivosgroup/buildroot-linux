/*****************************************************************
**                                                              **
**  This file is created by AfControlWizard.                    **
**  Do Not Edit it with Editor or Merge tool !!!                **
**  Any Editing May make AfControlWizard fail to open it        **
**                                                              **
*****************************************************************/

/*****************************************************************
**                                                              **
**  Copyright (C) 2007 Amlogic,Inc.                             **
**  All rights reserved                                         **
**                                                              **
*****************************************************************/
#include "uBttvApp.h"

#define PROP_TRANSFERMANAGER_POS 0

#define PROP_DOWNLOADPATH_POS 1

#define PROP_SERVERURL_POS 2

#define PROP_ROOTURL_POS 3

#define PROP_SEARCHURL_POS 4

#define PROP_SEARCHBTURL_POS 5

#define PROP_SEARCHTYPE_POS 6

#define PROP_KANKANNAVIURL_POS 7

#define PROP_KANKANSEARURL_POS 8

#define PROP_MOUNTINFO_POS 9

#define PROP_ERRORNO_POS 10

static int BttvApp_Init(control_t* cntl, cond_item_t* param);

static int BttvApp_UnInit(control_t* cntl, cond_item_t* param);

static int BttvApp_MsgProcess(control_t* cntl, cond_item_t* param);

/*static int BttvApp_SerialNumberCheck(control_t* cntl, cond_item_t* param)
{
    return 8;
}

static int BttvApp_RequestData(control_t* cntl, cond_item_t* param);

static int BttvApp_RequestPicture(control_t* cntl, cond_item_t* param);

static int BttvApp_CancelRequest(control_t* cntl, cond_item_t* param);

static int BttvApp_GetErrorMsg(control_t* cntl, cond_item_t* param);

static int BttvApp_SetDemoModel(control_t* cntl, cond_item_t* param);

static int BttvApp_SetDownlodPath(control_t* cntl, cond_item_t* param);

static int BttvApp_CurDLPathSpace(control_t* cntl, cond_item_t* param);

static int BttvApp_SetSearchType(control_t* cntl, cond_item_t* param);
*/
extern int BttvApp_SetSearchType(control_t* cntl, cond_item_t* param);
/*
method_func BttvApp_Method_Set[]=
{
BttvApp_Init,
BttvApp_UnInit,
BttvApp_MsgProcess,
BttvApp_SerialNumberCheck,
BttvApp_RequestData,
BttvApp_RequestPicture,
BttvApp_CancelRequest,
BttvApp_GetErrorMsg,
BttvApp_SetDemoModel,
BttvApp_SetDownlodPath,
BttvApp_CurDLPathSpace,
BttvApp_SetSearchType,
};
*/
#define TransferInter_PROP_task_table  "task_table"

#define TransferInter_PROP_cb_handler  "cb_handler"

#define TransferInter_PROP_last_error  "last_error"

#define TransferInter_METHOD_SetOption  "SetOption"

#define TransferInter_METHOD_GetOption  "GetOption"

#define TransferInter_METHOD_Init_Ex  "Init_Ex"

#define TransferInter_METHOD_UpdateTaskInfo  "UpdateTaskInfo"

#define TransferInter_METHOD_UnloadAllTasks  "UnloadAllTasks"

#define TransferInter_METHOD_LoadAllTasks  "LoadAllTasks"

#define TransferInter_METHOD_AddTask  "AddTask"

#define TransferInter_METHOD_GetResult  "GetResult"

#define TransferInter_METHOD_AddThunderTask  "AddThunderTask"

#define TransferInter_METHOD_StartTask  "StartTask"

#define TransferInter_METHOD_StartPrioTask  "StartPrioTask"

#define TransferInter_METHOD_PauseTask  "PauseTask"

#define TransferInter_METHOD_CancelTask  "CancelTask"

#define TransferInter_METHOD_CloseTask  "CloseTask"

#define TransferInter_METHOD_GetTaskList  "GetTaskList"

#define TransferInter_METHOD_GetTaskInfo  "GetTaskInfo"

#define TransferInter_METHOD_ReservedSpace  "ReservedSpace"

#define TransferInter_METHOD_SetInfoPath  "SetInfoPath"

#define TransferInter_METHOD_SetTempPath  "SetTempPath"

#define TransferInter_METHOD_GetSeedInfo  "GetSeedInfo"

#define TransferInter_METHOD_SetStatCallback  "SetStatCallback"

#define TransferInter_METHOD_EnableCacheFile  "EnableCacheFile"

#define TransferInter_METHOD_SetCustomAlloc  "SetCustomAlloc"

#define TransferInter_METHOD_Fini  "Fini"

#define TransferInter_METHOD_SetLicense  "SetLicense"

#define TransferInter_METHOD_GetThdTaskid  "GetThdTaskid"

#define TransferInter_METHOD_PauseAllTask  "PauseAllTask"

#define TransferInter_METHOD_RestartAllTask  "RestartAllTask"

#define TransferInter_METHOD_CloseAllTask  "CloseAllTask"

#define TransferInter_METHOD_CancelAllTask  "CancelAllTask"

#define TransferInter_METHOD_GetLastError  "GetLastError"

#define StringTable_PROP_ColNickNameList  "ColNickNameList"

#define StringTable_METHOD_GetRowCount  "GetRowCount"

#define StringTable_METHOD_GetColCount  "GetColCount"

#define StringTable_METHOD_GetString  "GetString"

#define StringTable_METHOD_GetRowName  "GetRowName"

#define StringTable_METHOD_GetRowIndex  "GetRowIndex"

#define StringTable_METHOD_AddRow  "AddRow"

#define StringTable_METHOD_DelRow  "DelRow"

#define StringTable_METHOD_SetString  "SetString"

#define StringTable_METHOD_DelString  "DelString"

#define StringTable_METHOD_Release  "Release"

/*
<<ControlName>> BttvApp

<<AfControlWizardVersion>> 00000227
*/

/*
<<Magic>> 0006142b
*/
