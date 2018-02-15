#include "ModbusSend.h"

/*  
CRC算法名称	    多项式公式	宽度	多项式	初始值	结果异或值	输入值反转	输出值反转
CRC-16/MODBUS	16+x15+x2+1	16	    8005	FFFF	0000	    true	    true
*/
ushort CRC16(uchar *Msg, ushort DataLen)
{
    uchar uchCRCH = 0xFF; /* 高位CRC字节初始化 */
    uchar uchCRCL = 0xFF; /* 低位CRC字节初始化 */
    uint uIndex; /* CRC循环中的索引 */

    while (DataLen--) { /* 传输消息缓冲区 */
        uIndex = uchCRCH ^ *Msg++; /* 计算CRC */
        uchCRCH = uchCRCL ^ CRCH[uIndex];
        uchCRCL = CRCL[uIndex];
    }

    return (uchCRCH<<8 | uchCRCL);
}

/* 判断串口接收 */
int CheckSerailMsg(uchar *sbuf) 
{
    ushort crcData;
    crcData = CRC16(sbuf, 6);
    if (crcData == (sbuf[7]+(sbuf[6]<<8))) {//校验正确
        if (sbuf[1] == 0x03) {
            return 1;
        } else {
            return -1;//功能码错误 
        }
    } else {
        return -2;//CRC校验错误 
    }
}
