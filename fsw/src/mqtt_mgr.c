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
**   None
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_mgr.h"


/**********************/
/** File Global Data **/
/**********************/

static void SubscribeToMessages(uint32 TopicBaseMid);


/*****************/
/** Global Data **/
/*****************/

static MQTT_MGR_Class_t* MqttMgr;


/******************************************************************************
** Function: MQTT_MGR_Constructor
**
** Initialize the MQTT Manager object
**
** Notes:
**   1. This must be classed prior to any other member functions.
**
*/
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *MqttMgrPtr,
                          const INITBL_Class_t *IniTbl, TBLMGR_Class_t *TblMgr)
{

   MgttMgr = MqttMgrPtr;
   
   memset(MgttMgr, 0, sizeof( MQTT_MGR_Class_t));
   
   MgttMgr->MqttYieldTime = INITBL_GetIntConfig(IniTbl, CFG_MQTT_CLIENT_YIELD_TIME);
   MgttMgr->SbPendTime    = INITBL_GetIntConfig(IniTbl, CFG_TOPIC_PIPE_PEND_TIME);
   
   CFE_SB_CreatePipe(&MqttMgr->TopicPipe, INITBL_GetIntConfig(IniTbl, CFG_TOPIC_PIPE_DEPTH),
                     INITBL_GetStrConfig(IniTbl, CFG_TOPIC_PIPE_NAME));
   
   MQTT_CLIENT_Constructor(&MqttMgr->MqttClient, IniTbl);

   MSG_TRANS_Constructor(&MqttMgr->MsgTrans, IniTbl);

   SubcribeToMessages(INITBL_GetIntConfig(IniTbl, CFG_TOPIC_MSG_MID));
                  
} /* End MQTT_MGR_Constructor() */


/******************************************************************************
** Function: MQTT_MGR_ProcessSbTopics
**
** Pend with timeout for topic messages sent on the software bus
**
*/
void MQTT_MGR_ProcessSbTopics(uint32 PerfId)
{

   int32  SbStatus;
   CFE_SB_Buffer_t  *SbBufPtr;

   do 
   {
      CFE_ES_PerfLogExit(PerfId);
      SbStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, MqttMgr.TopicPipe, MgttMgr->SbPendTime);
      CFE_ES_PerfLogEntry(PerfId);
   
      if (SbStatus == CFE_SUCCESS)
      {
         MSG_TRANS_ProcessSbMsg(&SbBufPtr->Msg);
      }
      
   } while(SbStatus == CFE_SUCCESS);
   
} /* End MQTT_MGR_ProcessSbTopics() */


/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr)
{

   MQTT_CLIENT_Yield(MgttMgr->MqttYieldTime);

} /* End MQTT_MGR_ChildTaskCallback();


/******************************************************************************
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void)
{

   MQTT_CLIENT_ResetStatus();
   MSG_TRANS_ResetStatus();

} /* End MQTT_MGR_ResetStatus() */


/******************************************************************************
** Function: SubscribeToMessages
**
** Subscribe to topic messages on the SB and MQTT_CLIENT based on a topics
** defition in the topic table.
**
*/
static void SubscribeToMessages(uint32 TopicBaseMid)
{

   uint16 i;
   uint16 SbSubscribeCnt = 0;
   uint16 MqttSubscribeCnt = 0;
   uint16 SubscribeErr = 0;
   
   MQTT_TOPIC_TBL_Entry_t TopicTblEntry;
   
   for (i=0; i < MQTT_TOPIC_TBL_MAX_TOPICS; i++)
   {
   
      TopicTblEntry = MQTT_TOPIC_TBL_GetEntry(i);
      if (TopicTblEntry->Id != MQTT_TOPIC_TBL_UNUSED_ID)
      {
         if (strcmp(TopicTblEntry->SbRole,"pub") == 0)
         {
            ++SbSubscribeCnt;
            CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TopicBaseMid+i),
                             MqttMgr->TopicPipe);
         }
         else
         {
            /* MQTTlib does not store a copy of topic so it must be in persistent memory */
            if MQTT_CLIENT_Subscribe(TopicTblEntry->Name, MQTT_CLIENT_QOS2, 
                                  MSG_TRANS_ProcessMqttMsg)
            {
               ++MqttSubscribeCnt;
            }
            else
            {
               ++SubscribeErr;
               CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_INFORMATION, 
                       "Error subscrining to MQTT client for topic %s", TopicTblEntry->Name);
            }
         }
      } /* End if toopic in use */
  
   } /* End topic loop */
    
   CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                     "Subscribed to %d topics on SB and %d topics on MQTT with %d errors",
                     SbSubscribeCnt, MqttSubscribeCnt, SubscribeErr);
 
} /* End SubscribeToMessages() */
