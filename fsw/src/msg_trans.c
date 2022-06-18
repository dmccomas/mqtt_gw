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
**   Translate between MQTT and Software Bus messages
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

/*
** Include Files:
*/

#include <string.h>

#include "msg_trans.h"

/**********************/
/** Global File Data **/
/**********************/

static MSG_TRANS_Class_t *MsgTrans = NULL;


/******************************************************************************
** Function: MSG_TRANS_Constructor
**
*/
void MSG_TRANS_Constructor(MSG_TRANS_Class_t *MsgTransPtr,
                           const INITBL_Class_t  *IniTbl,
                           TBLMGR_Class_t *TblMgr)
{
 
   MsgTrans = MsgTransPtr;

   CFE_PSP_MemSet((void*)MsgTransPtr, 0, sizeof(MSG_TRANS_Class_t));

   MsgTrans->TopicBaseMid  = INITBL_GetIntConfig(IniTbl, CFG_TOPIC_MSG_MID);
   
   MQTT_TOPIC_TBL_Constructor(&MsgTrans->TopicTbl, 
                              INITBL_GetStrConfig(IniTbl, CFG_APP_CFE_NAME),
                              MsgTrans->TopicBaseMid);
                              
   TBLMGR_RegisterTblWithDef(TblMgr, MQTT_TOPIC_TBL_LoadCmd, 
                             MQTT_TOPIC_TBL_DumpCmd,  
                             INITBL_GetStrConfig(IniTbl, CFG_MQTT_TOPIC_TBL_DEF_FILE));                           


} /* End MSG_TRANS_Constructor() */


/******************************************************************************
** Function: MSG_TRANS_ProcessMqttMsg
**
** Notes:
**   1. Signature must match MQTT_CLIENT_MsgCallback_t
**   2. MQTT has no delimeter between the topic and payload
**
*/
void MSG_TRANS_ProcessMqttMsg(MessageData* MsgData)
{
   
   MQTTMessage* MsgPtr = MsgData->message;

   int16 i = 0;
   int   TopicLen;
   char  TopicStr[MQTT_TOPIC_TBL_MAX_TOPIC_LEN];
   bool  MsgFound = false;
   const MQTT_TOPIC_TBL_Entry_t *TopicTblEntry;
   MQTT_TOPIC_TBL_JsonToCfe_t JsonToCfe;
   CFE_MSG_Message_t **CfeMsg;
   
OS_printf("\n****************************  MSG_TRANS_ProcessMqttMsg() ****************************\n");
   CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION,
                     "MSG_TRANS_ProcessMsg: Received topic %s", MsgData->topicName->lenstring.data);
                    
   if(MsgPtr->payloadlen)
   {
      
      OS_printf("MsgPtr->payloadlen=%d\n", (int)MsgPtr->payloadlen);

      while ( MsgFound == false && i < MQTT_TOPIC_TBL_MAX_TOPICS)
      {

        TopicTblEntry = MQTT_TOPIC_TBL_GetEntry(i);
        if (TopicTblEntry != NULL)
        {
            
            TopicLen = strlen(TopicTblEntry->Name);
            
            OS_printf("Table topic name=%s, length=%d\n", TopicTblEntry->Name, TopicLen);
            if (strncmp(TopicTblEntry->Name, MsgData->topicName->lenstring.data, TopicLen) == 0)
            {
               MsgFound = true;
               memcpy(TopicStr, MsgData->topicName->lenstring.data, TopicLen);
               TopicStr[TopicLen] = '\0';
               CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION,
                                "MSG_TRANS_ProcessMqttMsg: Topic=%s, Payload=%s", 
                                 TopicStr, &MsgData->topicName->lenstring.data[TopicLen]);
            } 
            else
            {
               i++;
            }
        }
      } /* End while loop */

      if (MsgFound)
      {
            
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION,
                           "MSG_TRANS_ProcessMqttMsg: Found message at index %d", i); 
                       
         JsonToCfe = MQTT_TOPIC_TBL_GetJsonToCfe(i);    
         
         if (JsonToCfe(CfeMsg, &MsgData->topicName->lenstring.data[TopicLen]))
         {
      
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION,
                              "MSG_TRANS_ProcessMqttMsg: Found message at index %d", i); 
            
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(CfeMsg));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(CfeMsg), true);

         }
         else
         {
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION,
                              "MSG_TRANS_ProcessMqttMsg: Error creating SB message from JSON"); 
                     
         }
         
      } /* End if message found */
      else 
      {
      
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_INFORMATION, 
                           "MSG_TRANS_ProcessMqttMsg: Could not find a topic match for %s", 
                           MsgData->topicName->lenstring.data);
      
      }
   
   } /* End null message len */
   else {
      
      OS_printf("Null MQTT wessage data length for %s\n", MsgData->topicName->lenstring.data);
    
   }
   
   fflush(stdout);

  
} /* End MSG_TRANS_ProcessMqttMsg() */


/******************************************************************************
** Function: MSG_TRANS_ProcessSbMsg
**
** Notes:
**   None
**
*/
void MSG_TRANS_ProcessSbMsg(const CFE_MSG_Message_t *MsgPtr)
{

   int32 TopicId;
   int32 SbStatus;
   CFE_SB_MsgId_t  MsgId = CFE_SB_INVALID_MSG_ID;

   OS_printf("\n****************************  MSG_TRANS_ProcessSBMsg() ****************************\n");

   SbStatus = CFE_MSG_GetMsgId(MsgPtr, &MsgId);
   if (SbStatus == CFE_SUCCESS)
   {
   
      //TODO - Call translator
      TopicId = CFE_SB_MsgIdToValue(MsgId) - MsgTrans->TopicBaseMid;
      if (TopicId >= 0 && TopicId < MQTT_TOPIC_TBL_MAX_TOPICS)
      {
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_INFORMATION, 
                           "MSG_TRANS_ProcessSbMsg: Received topic id %d", TopicId); 
      }
      else
      {
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR, 
                           "MSG_TRANS_ProcessMsg: Computed invalid topic id  %d from received MID 0x%04X and base MID 0x%04X", 
                           TopicId, CFE_SB_MsgIdToValue(MsgId), MsgTrans->TopicBaseMid);
      }

   }

} /* End MSG_TRANS_ProcessSbMsg() */


/******************************************************************************
** Function: MSG_TRANS_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MSG_TRANS_ResetStatus(void)
{

   MQTT_TOPIC_TBL_ResetStatus();

} /* MSG_TRANS_ResetStatus() */


