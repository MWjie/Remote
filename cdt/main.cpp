#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include "simscreen.h"
#include "cdt.h"
#include "ModbusTCPCli.h"

extern CRITICAL_SECTION CriticalSection;

int main(int argc,char **argv)
{
    //��ʼ���ٽ������� 
    InitializeCriticalSection(&CriticalSection);
    //�������� 
    loadCfg("setting.cfg");
   
    /* ԭģ�������� */
//	screenAllOpen();//Inited all open
//	screenAllLight();//Light up the screen
//	startModbusScan();//Start modbus scan 
//	startYxCdtBroadCast();//Start cdt broadcast 
    /* ��Modbus���ڷ��ͳ��� */
    startModbusScan();//Start modbus scan 
    startModbusSerail();//start modbus serail send 
}
