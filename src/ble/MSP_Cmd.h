
//----------------------------------------------------------------------------
//    프로그램명 	: MSP Cmd
//
//    만든이     	: Cho Han Cheol
//
//    날  짜     :
//    
//    최종 수정  	:
//
//    MPU_Type	: 
//
//    파일명     	: MSP_Cmd.h
//----------------------------------------------------------------------------
#ifndef _MSP_CMD_H_
#define _MSP_CMD_H_

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>



#define MSP_SET_RAW_RC_TINY      150   //in message          4 rc chan
#define MSP_ARM                  151
#define MSP_DISARM               152
#define MSP_TRIM_UP              153
#define MSP_TRIM_DOWN            154
#define MSP_TRIM_LEFT            155
#define MSP_TRIM_RIGHT           156

#define MSP_TRIM_UP_FAST         157
#define MSP_TRIM_DOWN_FAST       158
#define MSP_TRIM_LEFT_FAST       159
#define MSP_TRIM_RIGHT_FAST      160

#define MSP_READ_TEST_PARAM      189
#define MSP_SET_TEST_PARAM       190

#define MSP_READ_TEST_PARAM      189
#define MSP_HEX_NANO             199



#define MSP_CMD_MAX_LENGTH		64


typedef struct 
{
	uint8_t	Cmd;
	uint8_t	Length;
	uint8_t CheckSum;
	uint8_t	Data[MSP_CMD_MAX_LENGTH];	
} MSP_CMD_OBJ;


typedef struct 
{
	uint8_t	Cmd;
	uint8_t	Length;
	uint8_t ErrorCode;
	uint8_t CheckSum;
	uint8_t	Data[MSP_CMD_MAX_LENGTH];	
} MSP_RESP_OBJ;



typedef struct
{
	uint8_t	Length;
	uint8_t	Data[MSP_CMD_MAX_LENGTH];
} MSP_BUF_OBJ;


void MSP_Init( void );
bool MSP_Update( uint8_t ch );

MSP_CMD_OBJ  *MSP_Get_CmdPtr( void );
MSP_RESP_OBJ *MSP_Get_RespPtr( void );
MSP_BUF_OBJ  *MSP_SendRespBuf( MSP_RESP_OBJ *pResp );



#endif
