#ifndef CDT_H
#define CDT_H

#include "type.h"

#define YX_RED    ( 0x01 )//红灯 合-1 
#define YX_GREEN  ( 0x02 )//绿灯 分-0 
#define YX_CLOSE  ( 0x00 )//灭灯
#define YX_YELLOW ( 0x03 )//黄灯	

#define SCREEN_ALL_CLOSE      ( 0x03 )//(全屏合)
#define SCREEN_ALL_OPEN       ( 0x04 )//(全屏分)
#define SCREEN_ALL_BLUCK      ( 0x07 )//(全屏暗盘)
#define SCREEN_ALL_NIGHT      ( 0x06 )//(全屏亮盘)
#define SCREEN_ALL_YX_UPDATE  ( 0x0A )//(YX更新)
#define SCREEN_ALL_RECOVERY   ( 0x01 )//01(操作恢复)

#define MAX_YX_SIZE_ONE_PACKAGE ( 249*4 )//单帧报文最大YX量个数 

extern char Buffer[];//通信缓冲

/*typedef int(*IO_CallBack)(char* buf, int size);
extern IO_CallBack ioWriteCallBack;
extern IO_CallBack ioReadCallBack;*/

#ifdef __cplusplus  
extern "C" {  
#endif 

void io_read(char* buf, int size, int selail_number);//由协议使用者实现 
void io_write(char* buf, int size, int selail_number);//由协议使用者实现

//注册回调函数
//void registerIoCallBack(IO_CallBack read,IO_CallBack write);

//计算校验 
uchar getcheck(char* buf, int len);
//成批YX
void sendBatchYX(ushort startSeq, uchar* yxs, int size, uint Serail_NUM);
void sendBatchYXNoLimit(ushort startSeq, uchar* yxs, int size, uint Serail_NUM);
//单个YX
void sendSingleYX(ushort seq, uchar yx, uint Serail_NUM);
//成批YC 
void sendBatchYC(ushort startSeq, uchar* ycs, int size, uint Serail_NUM);

//全屏操作 
void screenAllClose(uint Serail_NUM);
void screenAllOpen(uint Serail_NUM);
void screenAllBluck(uint Serail_NUM);
void screenAllLight(uint Serail_NUM);
void screenAllYxUpdate(uint Serail_NUM);
void screenAllRecovery(uint Serail_NUM);

//时间报文
void sendTime(ushort year, uchar month, uchar day, uchar week, uchar hour, uchar min, uchar sec, ushort safedays, uint Serail_NUM);

#ifdef __cplusplus  
}  
#endif 

#endif
