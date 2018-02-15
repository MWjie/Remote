#ifndef MODBUSTCPCLI_H
#define MODBUSTCPCLI_H

#include <iostream>
#include "CTcpClient.h"

#define MAX_COMMON_ERROR_CNT ( 3 )//最大通信错误次数 

class CModbusTcpCli: public CTcpClient {
private:
    //CTcpClient m_tcpcli;
public:
    char tag[25];//host+port 的标记 
    int linkStatus;//链接状态 
    int commonErrorCnt;//通信错误计数 
public:
    CModbusTcpCli(char* host, ushort port); 
    int open(void);
    int close(void);
    int readReg(uchar mbaddr, uchar func, ushort regaddr, uchar num, char* buf);	
    int writeReg(uchar mbaddr, uchar func, ushort regaddr, uchar num, char* buf);
};
void switchBytes(uchar* buf, int size);
void switchReg(ushort* buf, int size);
void reverseBit(uchar* buf, int size);
#endif
