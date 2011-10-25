/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Control Module
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

int transfer_ctrl_init();

void transfer_ctrl_fini();

void *transfer_ctrl_run(void *data);
