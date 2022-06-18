/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Manage MQTT interface
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topic's content.
**   2. The OSK JSON table coding ideom is use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_mgr_
#define _mqtt_mgr_

/*
** Includes
*/

#include "app_cfg.h"
#include "msg_trans.h"
#include "mqtt_client.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_MGR_SUBSCRIBE_EID      (MQTT_MGR_BASE_EID + 0)
#define MQTT_MGR_SUBSCRIBE_ERR_EID  (MQTT_MGR_BASE_EID + 1)


/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{


   uint32  MqttYieldTime;
   uint32  SbPendTime;
   
   CFE_SB_PipeId_t TopicPipe;
   
   /*
   ** Contained Objects
   */
   
   MQTT_CLIENT_Class_t  MqttClient;
   MSG_TRANS_Class_t    MsgTrans;  
   
} MQTT_MGR_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_MGR_Constructor
**
** Initialize the MQTT Manager object
**
** Notes:
**   1. This must be classed prior to any other member functions.
**
*/
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *TopicMgrPtr,
                          const INITBL_Class_t *IniTbl, TBLMGR_Class_t *TblMgr);


/******************************************************************************
** Function: MQTT_MGR_ProcessSbTopics
**
** Pend with timeout for topic messages sent on the software bus
**
*/
void MQTT_MGR_ProcessSbTopics(uint32 PerfId);


/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr);


/******************************************************************************
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void);


#endif /* _mqtt_mgr_ */
