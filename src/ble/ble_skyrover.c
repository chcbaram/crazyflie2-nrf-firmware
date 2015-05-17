/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie 2.0 NRF Firmware
 * Copyright (c) 2014, Bitcraze AB, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 *
 * Bluetooth LE Crazyflie service
 *
 * Bitcraze UUID base: 00000000-1c7f-4f9e-947b-43b7c00a9a08
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ble.h>

#include "skyrover_uuids.h"

#include "esb.h"
#include "ble_skyrover.h"
#include "MSP_Cmd.h"
#include "led.h"

uint8_t CmdBuf[128];

volatile MSP_CMD_OBJ    *pCmd;
volatile MSP_RESP_OBJ   *pResp;


void bleSkyRoverSendMsp( MSP_BUF_OBJ *pBuf );

void MSP_Cmd_MSP_SET_RAW_RC_TINY( void );



static ble_gatts_char_handles_t skyrover_handle;
static uint16_t mConnHandle = 0xffffu;

static ble_gatts_char_md_t skyrover_md = {
  .char_props = { .read = 1, .write = 1, .notify = 1 },
  //.char_ext_props = {},
  .p_char_user_desc = (uint8_t *) "HMSoft",
  .char_user_desc_max_size = 32,
  .char_user_desc_size = 6,

  .p_char_pf = NULL,
  .p_sccd_md = NULL,
};


static ble_gatts_attr_md_t skyrover_attr_md = {
  .read_perm = {.sm = 1, .lv = 1},
  .write_perm = {.sm = 1, .lv = 1},
  .vloc = BLE_GATTS_VLOC_STACK,
  .vlen = 1,
};

static ble_gatts_attr_t skyrover_attr = {
  .p_attr_md = &skyrover_attr_md,
  .init_len = 1,
  .init_offs = 0,
  .max_len = 32,
};



static bool crtpPacketReceived = false;
static EsbPacket rxPacket;

#define ERROR_CHECK(E) if (E != NRF_SUCCESS) { return err; }

void ble_skyrover_on_ble_evt(ble_evt_t * p_ble_evt)
{
  ble_gatts_evt_write_t *p_write;
  uint16_t i;
  bool ret = false;
  uint16_t length;

  switch(p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      mConnHandle = p_ble_evt->evt.gap_evt.conn_handle;
      break;
    case BLE_GAP_EVT_DISCONNECTED:
      mConnHandle = 0xffffu;
      break;
    case BLE_GATTS_EVT_WRITE:
      p_write = &p_ble_evt->evt.gatts_evt.params.write;
      if (p_write->handle == skyrover_handle.value_handle) {
        if (!crtpPacketReceived)
        {
          //memcpy(rxPacket.data, p_write->data, p_write->len);
          //rxPacket.size = p_write->len;
          if( p_write->len > 128 ) 	length = 128;
          else						length = p_write->len;

          memcpy(CmdBuf, p_write->data, length);

          MSP_Init();
          for( i=0; i<length; i++ )
          {
        	  ret = MSP_Update( CmdBuf[i]);
          }

          crtpPacketReceived = ret;
        }
      }
      break;
    default:
      break;
  }
}


uint32_t ble_skyrover_init(uint8_t uuidType)
{
  uint32_t err;
  ble_uuid_t  service_uuid;
  ble_uuid_t  crtp_uuid;
  uint16_t service_handle;
  uint8_t initial_value = 0xFF;
  ble_gatts_attr_md_t cccd_md;

  service_uuid.uuid = UUID_SKYROVER_SERVICE;
  service_uuid.type = uuidType;

  err = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 &service_uuid,
                                 &service_handle);
  ERROR_CHECK(err);

  /* Bidirectional full-length CRTP characteristic  */
  memset(&cccd_md, 0, sizeof(cccd_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

  skyrover_md.p_cccd_md  = &cccd_md;


  crtp_uuid.type = uuidType;
  crtp_uuid.uuid = UUID_SKYROVER_COMM;

  skyrover_attr.p_uuid = &crtp_uuid;
  skyrover_attr.p_value = &initial_value;
  skyrover_attr.init_len = 1;

  err = sd_ble_gatts_characteristic_add(service_handle,
                                        &skyrover_md,
                                        &skyrover_attr,
                                        &skyrover_handle);

  ERROR_CHECK(err);

  return NRF_SUCCESS;
}


bool bleSkyRoverIsPacketReceived()
{
	MSP_BUF_OBJ *SendBuf;

	if( crtpPacketReceived == true )
	{
	    //-- 명령어 처리
	    //

        pCmd  = MSP_Get_CmdPtr();
        pResp = MSP_Get_RespPtr();

        switch( pCmd->Cmd )
        {
            case MSP_SET_RAW_RC_TINY:
                MSP_Cmd_MSP_SET_RAW_RC_TINY();
                break;

            case MSP_ARM:
                break;

            case MSP_DISARM:
                break;

            default:
                pResp->Cmd       = pCmd->Cmd;
                pResp->ErrorCode = 0x00;
                pResp->Length    = 0;
                SendBuf = MSP_SendRespBuf( pResp );
                bleSkyRoverSendMsp( SendBuf );
                break;
	    }

        LED_TOGGLE();
	}



	return crtpPacketReceived;
}

EsbPacket* bleSkyRoverGetRxPacket()
{
  return &rxPacket;
}


void bleSkyRoverReleaseRxPacket(EsbPacket* packet)
{
  crtpPacketReceived = false;
}

/*
void bleSkyRoverSendPacket(EsbPacket* packet)
{
  ble_gatts_hvx_params_t params;
  uint16_t len = packet->size;
  static unsigned char buffer[20];
  static int pid = 0;

  if (mConnHandle == 0xffffu)
    return;


  if (len>20)
    len = 20;

  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = crtp_handle.value_handle;
  params.p_data = packet->data;
  params.p_len = &len;

  sd_ble_gatts_hvx(mConnHandle, &params);

  // Send segmented packet
  len = (packet->size>19)?20:packet->size+1;

  buffer[0] = 0x80u | ((pid<<5)&0x60) | ((packet->size-1) & 0x1f);
  memcpy(&buffer[1], packet->data, len);

  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = crtpdown_handle.value_handle;
  params.p_data = buffer;
  params.p_len = &len;
  sd_ble_gatts_hvx(mConnHandle, &params);

  if (packet->size > 19) {
    len = (packet->size-19)+1;

    // Continuation contains only the PID
    buffer[0] = ((pid<<5)&0x60);
    memcpy(&buffer[1], &packet->data[19], len);

    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = crtpdown_handle.value_handle;
    params.p_data = buffer;
    params.p_len = &len;
    sd_ble_gatts_hvx(mConnHandle, &params);
  }

  pid++;
}
*/


void bleSkyRoverSendMsp( MSP_BUF_OBJ *pBuf )
{
  ble_gatts_hvx_params_t params;
  uint16_t len = pBuf->Length;

  if (mConnHandle == 0xffffu)
    return;


  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = skyrover_handle.value_handle;
  params.p_data = pBuf->Data;
  params.p_len = &len;

  sd_ble_gatts_hvx(mConnHandle, &params);
}





void MSP_Cmd_MSP_SET_RAW_RC_TINY( void )
{
    volatile int16_t Roll;
    volatile int16_t Pitch;
    volatile int16_t Yaw;
    volatile int16_t Throthle;

    volatile uint8_t Aux1;
    volatile uint8_t Aux2;
    volatile uint8_t Aux3;
    volatile uint8_t Aux4;


    Roll     = 1000 + pCmd->Data[0] * 4;
    Pitch    = 1000 + pCmd->Data[1] * 4;
    Yaw      = 1000 + pCmd->Data[2] * 4;
    Throthle = 1000 + pCmd->Data[3] * 4;

    Aux1 = ( pCmd->Data[4] >> 6 ) & 0x03;
    Aux2 = ( pCmd->Data[4] >> 4 ) & 0x03;
    Aux3 = ( pCmd->Data[4] >> 2 ) & 0x03;
    Aux4 = ( pCmd->Data[4] >> 0 ) & 0x03;

    /*
        pCmd->Data[0] : 0~250 - Roll 값
        pCmd->Data[1] : 0~250 - Pitch 값
        pCmd->Data[2] : 0~250 - Yaw 값
        pCmd->Data[3] : 0~250 - Throttle 값
        pCmd->Data[4] : 0~255 - Aux 값
            7:6  Aux1
            5:4  Aux2
            3:2  Aux3
            1:0  Aux4

        Aux1
            - 0 : Headfree Mode Off
            - 2 : Headfree Mode On
        Aux2
            - 0 : 고도홀드 Off
            - 2 : 고도홀드 On
    */
}

