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

#ifndef _mqtt_topic_rate_
#define _mqtt_topic_rate_

/*
** Includes
*/

#include "app_cfg.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID (MQTT_TOPIC_RATE_BASE_EID + 0)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
*/

typedef struct
{

   float  X;
   float  Y;
   float  Z;

} MQTT_TOPIC_RATE_Data_t;

typedef struct
{
   
   CFE_MSG_TelemetryHeader_t  TelemetryHeader;
   MQTT_TOPIC_RATE_Data_t     Payload;

} MQTT_TOPIC_RATE_TlmMsg_t;


typedef struct
{

   /*
   ** Rate Telemetry
   */
   
   MQTT_TOPIC_RATE_TlmMsg_t  TlmMsg;
   char                      JsonMsgPayload[1024];
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the OSK
   ** table manager service, but is using core-json in the same way as an OSK
   ** table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_TOPIC_RATE_Class_t;


/************************/
/** Exported Functions **/
/************************/


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
                                 CFE_SB_MsgId_t TlmMsgMid);


/******************************************************************************
** Function: MQTT_TOPIC_RATE_CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
*/
bool MQTT_TOPIC_RATE_CfeToJson(char **JsonPayload, const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_RATE_JsonToCfe
**
** Convert a JSON rate topic message to a cFE rate message 
**
*/
bool MQTT_TOPIC_RATE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload);


#endif /* _mqtt_topic_rate_ */
