#ifndef SIMSCREEN_H
#define SIMSCREEN_H

#include "type.h"
#include "ModbusTCPCli.h"

#define MAX_REG_NUM ( 65535 )

typedef struct {
    char comport[512];
    int baud;
    ushort databit;
    ushort check;
    ushort stopbit;
    ushort devScanInterval;
    ushort simFlushT;
    ushort res;
} TY_SERAIL_PARAM;

//遥信量映射 
typedef struct {
    ushort yxSeqStart;//遥信量起始序号
    ushort yxSize;//遥信量个数 
    ushort mbRegStart;//modbus寄存器起始地址
    ushort mbRegSize;//modbus寄存器个数 
    ushort bit_start;//起始位从0开始编号，255表示不使用位 
    ushort bit_move;//位偏移量 
    ushort mbDevAddr;//modbus设备地址 
    ushort dataType;//数据类型  
    char tag[25];//ip+port标记 
    ushort res;//
} TY_YX_MB_MAP;
struct TY_YX_MB_MAP_LINK;
typedef struct TY_YX_MB_MAP_LINK* pTY_YX_MB_MAP_LINK;
typedef struct TY_YX_MB_MAP_LINK {
    TY_YX_MB_MAP* pYxMbMap;
    pTY_YX_MB_MAP_LINK next;
} TY_YX_MB_MAP_LINK;

//遥测量映射 
typedef struct {
    ushort ycSeqStart;//遥信量起始序号
    ushort ycSize;//遥信量个数 
    ushort mbRegStart;//modbus寄存器起始地址
    ushort mbRegSize;//modbus寄存器个数 
    ushort bit_start;//起始位从0开始编号，255表示不使用位 
    ushort bit_move;//位偏移量 
    ushort mbDevAddr;//modbus设备地址 
    ushort dataType;//数据类型 
    ushort byteorder;//字节顺序  
    char tag[25];//ip+port标记   
    ushort res;//
} TY_YC_MB_MAP;
struct TY_YC_MB_MAP_LINK;
typedef struct TY_YC_MB_MAP_LINK* pTY_YC_MB_MAP_LINK;
typedef struct TY_YC_MB_MAP_LINK {
    TY_YC_MB_MAP* pYcMbMap;
    pTY_YC_MB_MAP_LINK next;
} TY_YC_MB_MAP_LINK;

//经过计算式获取的遥测量重映射 
typedef struct {
    ushort ycModbusAddr;//遥测量MODBUS寄存器地址
    char formula[255]; //计算式 
} TY_YC_MB_REMAP;
struct TY_YC_MB_REMAP_LINK;
typedef struct TY_YC_MB_REMAP_LINK* pTY_YC_MB_REMAP_LINK;
typedef struct TY_YC_MB_REMAP_LINK {
    TY_YC_MB_REMAP* pYcMbREMap;
    pTY_YC_MB_REMAP_LINK next;
} TY_YC_MB_REMAP_LINK;

//modbus寄存器 
struct RegBlock;
typedef struct RegBlock *PRegBlock;
typedef struct RegBlock {
    uchar  func;
    uchar  res;
    ushort s_add;
    ushort size;
    PRegBlock next;
} TY_REG_BLOCK;

//modbus设备 
typedef struct {
    uchar mbAddr;//modbus地址 
    uchar transm;//通信方式 0-串口 1-网口 
    int fd;//通信句柄 
    char ip[16];//ip地址
    ushort port;//端口号
    char tag[25];//ip+port标记 
    TY_REG_BLOCK* pRegBlock; 
    ushort regVal[MAX_REG_NUM];
} TY_MB_DEVICE;

struct TY_MBDEV_LINK;
typedef struct TY_MBDEV_LINK* pTY_MBDEV_LINK;
typedef struct TY_MBDEV_LINK {
    TY_MB_DEVICE* pdev;	
    pTY_MBDEV_LINK next;
} TY_MBDEV_LINK;

//modbus tcp 实例
struct TY_MBTCPCLI_LINK;
typedef struct TY_MBTCPCLI_LINK* pTY_MBTCPCLI_LINK;
typedef struct TY_MBTCPCLI_LINK {
    char tag[25];//ip+port进行标示 
    CModbusTcpCli* pMbTcpCli;//modbus tcp cli 
    pTY_MBTCPCLI_LINK next;
} TY_MBTCPCLI_LINK;

#ifdef __cplusplus  
extern "C" {  
#endif 

void loadCfg(char* cfgfile);
//modbus扫描线程
void startModbusScan(void);
//yx CDT广播线程
void startYxCdtBroadCast(void);
//Modbus串口发送数据 
void startModbusSerail(void);
//根据设备地址和寄存器地址获取寄存器值 
int getMbRegValue(uchar mbaddr, ushort s_regaddr, int size, ushort byteorder, ushort* buf, char* tag); 

#ifdef __cplusplus  
}  
#endif 

#endif
