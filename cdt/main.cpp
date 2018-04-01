#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include "simscreen.h"
#include "cdt.h"
#include "ModbusTCPCli.h"

extern CRITICAL_SECTION CriticalSection;

int main(int argc,char **argv)
{
    //初始化临界区对象 
    InitializeCriticalSection(&CriticalSection);
    //加载配置 
    loadCfg("setting.cfg");
   
    /* 原模拟屏程序 */
//	screenAllOpen();//Inited all open
//	screenAllLight();//Light up the screen
//	startModbusScan();//Start modbus scan 
//	startYxCdtBroadCast();//Start cdt broadcast 
    /* 新Modbus串口发送程序 */
    startModbusScan();//Start modbus scan 
    startModbusSerail();//start modbus serail send 
}
