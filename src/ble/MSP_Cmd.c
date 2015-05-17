//----------------------------------------------------------------------------
//    프로그램명 	:
//
//    만든이     	: Cho Han Cheol
//
//    날  짜     :
//    
//    최종 수정  	:
//
//    MPU_Type	: 
//
//    파일명     	: MSP_Cmd.ino
//----------------------------------------------------------------------------



#include "MSP_Cmd.h"




#define MSP_CMD_START			'$'
#define MSP_CMD_HEADER_M		'M'
#define MSP_CMD_HEADER_ARROW 	'<'



#define MSP_CMD_STATE_WAIT_START			0
#define MSP_CMD_STATE_WAIT_HEADER_M			1
#define MSP_CMD_STATE_WAIT_HEADER_ARROW		2
#define MSP_CMD_STATE_WAIT_DATA_SIZE		3
#define MSP_CMD_STATE_WAIT_CMD				4
#define MSP_CMD_STATE_WAIT_DATA				5
#define MSP_CMD_STATE_WAIT_CHECKSUM			6





MSP_CMD_OBJ		Cmd;
MSP_RESP_OBJ	Resp;

MSP_BUF_OBJ		MSP_SendRespData;


MSP_CMD_OBJ		*MSP_Get_CmdPtr( void );
MSP_RESP_OBJ 	*MSP_Get_RespPtr( void );

void MSP_SendResp( MSP_RESP_OBJ *pResp );

static	uint8_t  Cmd_State = MSP_CMD_STATE_WAIT_START;
static	uint8_t  Index_Data;





/*---------------------------------------------------------------------------
     TITLE   : MSP_Cmd
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void MSP_Init( void )
{
	Cmd_State = MSP_CMD_STATE_WAIT_START;
}




/*---------------------------------------------------------------------------
     TITLE   : update
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP_Update( uint8_t ch )
{
	bool    Ret = false;


/*
	//-- 바이트간 타임아웃 설정(200ms)
	//
	CurrentTime = micros();

	if( (CurrentTime - PreviousTime) > 200000 )
	{
		Cmd_State    = MSP_CMD_STATE_WAIT_START;
		PreviousTime = CurrentTime;
	}
*/


	//-- 명령어 상태
	//
	switch( Cmd_State )
	{

		//-- 시작 문자 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_START:

			// 시작 문자를 기다림
			if( ch == MSP_CMD_START )
			{
				Cmd_State    = MSP_CMD_STATE_WAIT_HEADER_M;
			}
			break;


		//-- 'M' 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_HEADER_M:
			if( ch == MSP_CMD_HEADER_M )
			{
				Cmd_State = MSP_CMD_STATE_WAIT_HEADER_ARROW;				
			}
			else
			{
				Cmd_State = MSP_CMD_STATE_WAIT_START;
			}
			break;


		//-- '<' 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_HEADER_ARROW:
			if( ch == MSP_CMD_HEADER_ARROW )
			{
				Cmd.CheckSum = 0x00;
				Cmd.Length   = 0;				
				Cmd_State = MSP_CMD_STATE_WAIT_DATA_SIZE;				
			}
			else
			{
				Cmd_State = MSP_CMD_STATE_WAIT_START;
			}
			break;


				Cmd.CheckSum = 0x00;
				Cmd.Length   = 0;


		//-- 데이터 사이즈 기다리는 상태(64까지)
		//
		case MSP_CMD_STATE_WAIT_DATA_SIZE:

			if( ch <= MSP_CMD_MAX_LENGTH )
			{
				Cmd.Length    = ch;
				Index_Data    = 0;
				Cmd.CheckSum ^= ch;
				Cmd_State     = MSP_CMD_STATE_WAIT_CMD;
			}
			else
			{
				Cmd_State = MSP_CMD_STATE_WAIT_START;	
			}
			break;


		//-- 명령어를 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_CMD:

			Cmd.Cmd       = ch;
			Cmd.CheckSum ^= ch;

			if( Cmd.Length == 0 )
			{
				Cmd_State = MSP_CMD_STATE_WAIT_CHECKSUM;
			}
			else
			{
				Cmd_State = MSP_CMD_STATE_WAIT_DATA;
			}
			break;


		//-- 데이터를 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_DATA:
			
			Cmd.CheckSum          ^= ch;
			Cmd.Data[ Index_Data ] = ch;

			Index_Data++;

			if( Index_Data >= Cmd.Length )
			{
				Cmd_State = MSP_CMD_STATE_WAIT_CHECKSUM;
			} 
			break;


		//-- 체크섬을 기다리는 상태
		//
		case MSP_CMD_STATE_WAIT_CHECKSUM:

			if( Cmd.CheckSum == ch )
			{
				Ret = true;
			}

			Cmd_State = MSP_CMD_STATE_WAIT_START;		
			break;
	}

	return Ret;
}





/*---------------------------------------------------------------------------
     TITLE   : MSP_Get_CmdPtr
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
MSP_CMD_OBJ *MSP_Get_CmdPtr( void )
{
	return &Cmd;
}





/*---------------------------------------------------------------------------
     TITLE   : MSP_Get_RespPtr
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
MSP_RESP_OBJ *MSP_Get_RespPtr( void )
{
	return &Resp;
}





/*---------------------------------------------------------------------------
     TITLE   : MSP_SendRespBuf
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
MSP_BUF_OBJ *MSP_SendRespBuf( MSP_RESP_OBJ *pResp )
{
	uint8_t i;
	uint8_t CheckSum = 0;

	uint8_t Index;

	Index = 0;

	MSP_SendRespData.Data[Index++] = MSP_CMD_START;
	MSP_SendRespData.Data[Index++] = MSP_CMD_HEADER_M;


	if( pResp->ErrorCode > 0 )	MSP_SendRespData.Data[Index++] = '!';
	else						MSP_SendRespData.Data[Index++] = '>';

	MSP_SendRespData.Data[Index++] = pResp->Length;
	MSP_SendRespData.Data[Index++] = pResp->Cmd;
	CheckSum ^= pResp->Cmd;

	for( i=0; i<pResp->Length; i++ )
	{
		MSP_SendRespData.Data[Index++] = pResp->Data[i];

		CheckSum ^= pResp->Data[i];		
	}

	MSP_SendRespData.Data[Index++] = CheckSum;

	MSP_SendRespData.Length = Index;

	return &MSP_SendRespData;
}


