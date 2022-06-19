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
**   Manage MQTT rate topic
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

#include "mqtt_topic_rate.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_RATE_Class_t* MqttTopicRate = NULL;

static MQTT_TOPIC_RATE_Data_t RateData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                      core-json      length of query      */
   /* Data Address,   Data Length,  Updated, Data Type,  Float,  query string,  string(exclude '\0') */
   
   { &RateData.X,     4,            false,   JSONNumber, true,   { "rate.x",    (sizeof("rate.x")-1)} },
   { &RateData.Y,     4,            false,   JSONNumber, true,   { "rate.y",    (sizeof("rate.y")-1)} },
   { &RateData.Z,     4,            false,   JSONNumber, true,   { "rate.z",    (sizeof("rate.z")-1)} }
   
};


/******************************************************************************
** Function: MQTT_TOPIC_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
void MQTT_TOPIC_RATE_Constructor(MQTT_TOPIC_RATE_Class_t *MqttTopicRatePtr, 
                                 CFE_SB_MsgId_t TlmMsgMid)
{

   MqttTopicRate = MqttTopicRatePtr;
   memset(MqttTopicRate, 0, sizeof(MQTT_TOPIC_RATE_Class_t));

   MqttTopicRate->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicRate->TlmMsg), TlmMsgMid, 
                            sizeof(MQTT_TOPIC_RATE_TlmMsg_t));
   
} /* End MQTT_TOPIC_RATE_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_RATE_CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
*/
bool MQTT_TOPIC_RATE_CfeToJson(char **JsonPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_TOPIC_RATE_TlmMsg_t *RateMsg = (MQTT_TOPIC_RATE_TlmMsg_t*)CfeMsg;
   
   *JsonPayload = NULL;
   
   PayloadLen = sprintf(MqttTopicRate->JsonMsgPayload,
                "{\"rate\":{\"x\": %0.6f,\"y\": %0.6f,\"z\": %0.6f}}",
                RateMsg->Payload.X, RateMsg->Payload.Y, RateMsg->Payload.Z);

   if (PayloadLen > 0)
   {
      *JsonPayload = MqttTopicRate->JsonMsgPayload;
   
      ++MqttTopicRate->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_RATE_CfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_RATE_JsonToCfe
**
** Convert a JSON rate topic message to a cFE rate message 
**
*/
bool MQTT_TOPIC_RATE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttTopicRate->TlmMsg;

      ++MqttTopicRate->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_RATE_JsonToCfe() */



/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&MqttTopicRate->TlmMsg.Payload, 0, sizeof(MQTT_TOPIC_RATE_Data_t));
OS_printf("LoadJsonData() %s, %d\n",JsonMsgPayload, PayloadLen);
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicRate->JsonObjCnt, 
                                  JsonMsgPayload, PayloadLen);

   if (ObjLoadCnt == MqttTopicRate->JsonObjCnt)
   {

      memcpy(&MqttTopicRate->TlmMsg.Payload, &RateData, sizeof(MQTT_TOPIC_RATE_Data_t));
      RetStatus = true;
   
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing rate topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicRate->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

