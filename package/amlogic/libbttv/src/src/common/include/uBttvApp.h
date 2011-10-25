#ifndef UBTTVAPP_H
#define UBTTVAPP_H
 extern "C" int BttvApp_SerialNumberCheck(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_RequestData(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_RequestPicture(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_CancelRequest(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_GetErrorMsg(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_SetDemoModel(control_t* cntl, cond_item_t* param);

 extern "C" int BttvApp_SetDownlodPath(control_t* cntl, cond_item_t* param);

extern "C" int BttvApp_CurDLPathSpace(control_t* cntl, cond_item_t* param);

#endif // UBTTVAPP_H
