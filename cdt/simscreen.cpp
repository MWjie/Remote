#include "simscreen.h"
#include "cdt.h"
#include "serail.h"
#include "mystring.h"
#include "ModbusTCPCli.h"
#include "ModbusSend.h"
#include "calculate.h"
#include <Windows.h>

#define CFG_NOTES_FLAG        ( "#" )                         //注释标记 
#define CFG_COM_FLAG_END      ( "COM_END" )                   //加载串口配置结束标志 
#define CFG_DEVICE_FLAG       ( "DEV" )                       //设备定义标记 
#define CFG_DEVICE_FLAG_END   ( "DEV_END" )                   //设备定义结束标记 
#define CFG_YX_FLAG           ( "YX" )                        //遥信量定义标记
#define CFG_YX_FLAG_END       ( "YX_END" )                    //遥信量定义结束标记
#define CFG_YC_FLAG           ( "YC" )                        //遥测量定义标记
#define CFG_YC_FLAG_END       ( "YC_END" )                    //遥测量定义结束标记
#define CFG_YC_REMAP_FLAG     ( "YC_Formula" )                //遥测量重映射表标志
#define CFG_YC_REMAP_FLAG_END ( "Formula_END" )               //遥测量重映射表结束标志
#define CFG_MACHINE_ADDR      ( "The Address of The Machine" )//本机地址定义标志 
#define CFG_YX_START_SEQ 	  ( 0 )                           //遥信起始序号 
#define CFG_YC_START_SEQ      ( 4096 )                        //遥测起始序号

CSerail serail;
TY_MBDEV_LINK mbDevLink;
TY_YX_MB_MAP_LINK yxMbMapLink;
TY_YC_MB_MAP_LINK ycMbMapLink;
TY_YC_MB_REMAP_LINK ycMbREMapLink;
TY_MBTCPCLI_LINK mbTcpCliLink;
CRITICAL_SECTION CriticalSection;
TY_SERAIL_PARAM serailParam;
uchar yxFlag;//是否存在遥信量 
uchar ycFlag;//是否存在遥测量 
uchar MachineAddr;//主机地址 
volatile uint Serail_count;//串口个数 
volatile uint Serail_NUM[MAX_SERAIL_NUM];//保存已经存在的串口号 

void loadSerailParam(FILE* fp);
void loadMachineAddr(FILE* fp);
void loadDevice(FILE* fp);
void loadYxMap(FILE* fp);
void loadYcMap(FILE* fp);
void loadYcREMap(FILE* fp);
void addMbDevRegBlock(TY_REG_BLOCK* plink, TY_REG_BLOCK* pRegBlock);
void addMbDev(TY_MBDEV_LINK* plink, TY_MB_DEVICE* pDev);
void addYxMap(TY_YX_MB_MAP_LINK* plink, TY_YX_MB_MAP* pYxMap);
void addYcMap(TY_YC_MB_MAP_LINK* plink, TY_YC_MB_MAP* pYcMap);
void addYcREMap(TY_YC_MB_REMAP_LINK* plink, TY_YC_MB_REMAP* pYcREMap);

void openSerail(int serail_number)
{
    if (serail.open(serailParam.comport, serailParam.baud, serailParam.databit, serailParam.check, serailParam.stopbit, serail_number) != 0) {
        printf("open serail port: %d,%s,%d,%d,%d,%d failed\n", serail_number, serailParam.comport, serailParam.baud, serailParam.databit, serailParam.check, serailParam.stopbit);
        system("pause");
        exit(0);
    }
    serail.settimeout(200, 1000, 2000, 1000, 2000, serail_number);
}

void io_read(char* buf, int size, int serail_number)
{
    serail.read(buf, size, serail_number);
}
void io_write(char* buf, int size, int serail_number)
{
    serail.write(buf, size, serail_number);
    int i = 0;
    for (i=0; i<size; i++) {
        if (i%20==0 && i!=0) {
            printf("\n");
        }
        printf("%02X ", (uchar)*buf);
        buf++;
    }
    printf("\n\n");
}

//配置加载 
void loadCfg(char* cfgfile)
{
    FILE* fp = fopen(cfgfile, "r");
    if (!fp) {
        printf("open cfgfile error!\n");
        exit(0);
    }		
    //解析串口配置
    printf("loadSerailParam...\n");
    loadSerailParam(fp); 
    loadMachineAddr(fp);
    //解析设备定义
    printf("loadDevice...\n");
    loadDevice(fp);
    //解析遥信量定义
    loadYxMap(fp);	
    //解析遥测量定义
    loadYcMap(fp);	
    //解析遥测量重定义表 
    loadYcREMap(fp);
}

char* getRegBlock(char*p, uchar* pFunc, ushort* regaddr, uchar* num){
    char temp[20];
    char *ptemp;
    int i = 0;
    
    for (; i<3; i++) {
        ptemp = temp;
        while (*p != ',') {
            if (*p == '\0') {
                if (i < 2) {
                    return NULL;	
                }
                break;	
            }   
            *ptemp = *p;	
            ptemp++;
            p++;
        }
        p++;
        *ptemp = '\0';
        if (i == 0) {
            *pFunc = atoi(temp);
        } else if (i == 1) {
            *regaddr = atoi(temp);
        } else if (i == 2) {
            *num = atoi(temp);
        }
    } 
    return p;	
}
//打印设备列表 
void printMbDevList()
{
    pTY_MBDEV_LINK plink = &mbDevLink;
    while (plink != NULL) {
        printf("devs:%s\n", plink->pdev->tag);
        plink = plink->next;
    }
}

void loadSerailParam(FILE* fp)
{
    char buf[1024];
    Serail_count = 0;
    fscanf(fp, "%[^\n]%*c", serailParam.comport);
    while (1) {
        memset(serailParam.comport, 0, sizeof(serailParam.comport));
        fscanf(fp, "%s\n", buf);
        //判断是否串口配置结束 
        if (!strcmp(buf, CFG_COM_FLAG_END)) {
            break;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        }
        strcpy(serailParam.comport, buf);
        fscanf(fp, "%d%d%d%d%d%d",  &serailParam.baud,
                                    &serailParam.databit,
                                    &serailParam.check,
                                    &serailParam.stopbit,
                                    &serailParam.devScanInterval,
                                    &serailParam.simFlushT);
                                
        printf("%-6s%-6d%-6d%-6d%-6d%-6d%d\n",  serailParam.comport,
                                                serailParam.baud,
                                                serailParam.databit,
                                                serailParam.check,
                                                serailParam.stopbit,
                                                serailParam.devScanInterval,
                                                serailParam.simFlushT);
        Serail_NUM[Serail_count] = atoi(serailParam.comport+3);    	
        openSerail(Serail_NUM[Serail_count]);//打开串口
        Serail_count++;
    } 
}

void loadMachineAddr(FILE* fp)
{
    char buf[256];
    
    while (1) {
        memset(buf, 0, sizeof(buf));
        fscanf(fp, "%[^\n]%*c", buf);
        if (feof(fp)) {
            printf("no MachineAddr defined!\n");
            return;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        }
        //是否是主机地址定义 
        if (!strncmp(buf, CFG_MACHINE_ADDR, 26)) {
            break;
        }
    }
    MachineAddr = atoi(buf+28);
    printf("The Address of The Machine %d\n", MachineAddr);
}

void loadDevice(FILE* fp)
{
    char buf[2048];
    TY_REG_BLOCK* pRegBlock;
    
    //找头 
    while (1) {
        memset(buf, 0, sizeof(buf));
        fscanf(fp, "%s\n", buf);
        if (feof(fp)) {
            printf("no device defined!\n"); 
            return;
        }
        if (!strcmp(buf, CFG_DEVICE_FLAG)) {
            break;
        }
    }  
    //开始解析 
    TY_MB_DEVICE* dev;
    while (1) {
        memset(buf, 0, sizeof(buf)); 
        fscanf(fp, "%s", buf);
        
        //判断是否设备定义结束 
        if (!strcmp(buf, CFG_DEVICE_FLAG_END)) {
            break;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        } 
        
        //未结束则buf中数据是ip地址 
        dev = new TY_MB_DEVICE();
        strcpy(dev->ip, buf);
        
        memset(buf, 0, sizeof(buf));
        fscanf(fp, "%d%d%s", &dev->port, &dev->mbAddr, buf); 
        //解析寄存器块
        char *p = buf;
        uchar func;
        uchar num;
        ushort saddr;
        do {
            p = getRegBlock(p, &func, &saddr, &num);	
            if (p==NULL || p=='\0') {
                break;
            }
            pRegBlock = new TY_REG_BLOCK();
            pRegBlock->func  =func;
            pRegBlock->s_add = saddr;
            pRegBlock->size  = num;	           
            if(dev->pRegBlock == NULL) {
                dev->pRegBlock = pRegBlock;
            } else {
                addMbDevRegBlock(dev->pRegBlock, pRegBlock);
            }		   		   
        } while (1);
        addMbDev(&mbDevLink, dev);
    }
    //打印设备列表 
    printMbDevList();	
}

void loadYxMap(FILE* fp)
{
    char buf[2048];
    TY_YX_MB_MAP* pYxMbMap; 
    //找头 
    while (1) {
        memset(buf, 0, sizeof(buf));
        fscanf(fp, "%s\n", buf);
        if (feof(fp)) {
            printf("no yx defined!\n"); 
            printf("no yc defined!\n");
            yxFlag = 0;
            ycFlag = 0;
            return;
        }
        if (!strcmp(buf, CFG_YX_FLAG)) {
            yxFlag = 1;
            ycFlag = 0;
            break;
        }
        if (!strcmp(buf, CFG_YC_FLAG)) {//检测到没有遥信量而是遥测量不进入循环
            printf("no yx defined!\n");
            yxFlag = 0;
            ycFlag = 1; 
            break;
        }
    }  
 
       //开始解析
    while (yxFlag) {
        memset(buf, 0, sizeof(buf)); 
        fscanf(fp, "%s", buf);
        //判断是否设备定义结束 
        if (!strcmp(buf, CFG_YX_FLAG_END)) {
            return;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        } 
        pYxMbMap = new TY_YX_MB_MAP();
        //未结束则buf中数据是遥信起始序号
        pYxMbMap->yxSeqStart = atoi(buf);//实际从0开始编号 
        fscanf(fp,"%d%d%d%d%d%d%d%s\n", &pYxMbMap->yxSize,
                                        &pYxMbMap->mbRegStart,
                                        &pYxMbMap->mbRegSize,
                                        &pYxMbMap->bit_start,
                                        &pYxMbMap->bit_move,		                            
                                        &pYxMbMap->mbDevAddr,
                                        &pYxMbMap->dataType,
                                        &pYxMbMap->tag);
        printf("%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-12d%s\n", pYxMbMap->yxSeqStart,
                                                        pYxMbMap->yxSize,
                                                        pYxMbMap->mbRegStart,
                                                        pYxMbMap->mbRegSize,
                                                        pYxMbMap->bit_start,
                                                        pYxMbMap->bit_move,		                            
                                                        pYxMbMap->mbDevAddr,
                                                        pYxMbMap->dataType,
                                                        pYxMbMap->tag);
        addYxMap(&yxMbMapLink, pYxMbMap);		 
    } 	
}
void loadYcMap(FILE* fp)
{
    char buf[2048];
    TY_YC_MB_MAP* pYcMbMap; 
    //找头
    if (yxFlag) {//如果没有遥信而是遥测直接开始解析 
        while (1) {
            memset(buf, 0, sizeof(buf));
            fscanf(fp, "%s\n", buf);
            if (feof(fp)) {
                printf("no yc defined!\n"); 
                ycFlag = 0;
                return;
            }
            if (!strcmp(buf, CFG_YC_FLAG)) {
                ycFlag = 1;
                break;
            }
        }  
     } 
    //开始解析
    while (1) {
        memset(buf, 0, sizeof(buf)); 
        fscanf(fp, "%s", buf);
        //判断是否设备定义结束 
        if (!strcmp(buf, CFG_YC_FLAG_END)) {
            return;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        } 
        pYcMbMap = new TY_YC_MB_MAP();
        //未结束则buf中数据是遥信起始序号
        pYcMbMap->ycSeqStart = atoi(buf);//实际从0开始编号 
        fscanf(fp,"%d%d%d%d%d%d%d%d%s\n",   &pYcMbMap->ycSize,
                                            &pYcMbMap->mbRegStart,
                                            &pYcMbMap->mbRegSize,
                                            &pYcMbMap->bit_start,
                                            &pYcMbMap->bit_move,		                            
                                            &pYcMbMap->mbDevAddr,
                                            &pYcMbMap->dataType,
                                            &pYcMbMap->byteorder,
                                            &pYcMbMap->tag);
        printf("%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d%s\n",  pYcMbMap->ycSeqStart,
                                                            pYcMbMap->ycSize,
                                                            pYcMbMap->mbRegStart,
                                                            pYcMbMap->mbRegSize,
                                                            pYcMbMap->bit_start,
                                                            pYcMbMap->bit_move,		                            
                                                            pYcMbMap->mbDevAddr,
                                                            pYcMbMap->dataType,
                                                            pYcMbMap->byteorder,
                                                            pYcMbMap->tag);
        addYcMap(&ycMbMapLink, pYcMbMap);	
    } 	
}
void loadYcREMap(FILE* fp)
{
    char buf[2048];
    TY_YC_MB_REMAP* pYcMbREMap; 
    //找头
    while (1) {
        memset(buf, 0, sizeof(buf));
        fscanf(fp, "%s\n", buf);
        if (feof(fp)) {
            printf("no ycReMap defined!\n"); 
            return;
        }
        if (ycFlag != 1) {
            return; 	
        }
        if (!strcmp(buf, CFG_YC_REMAP_FLAG)) {
            break;
        }
    }  
    //开始解析
    while (1) {
        memset(buf, 0, sizeof(buf)); 
        fscanf(fp, "%s", buf);
        //判断是否设备定义结束 
        if (!strcmp(buf, CFG_YC_REMAP_FLAG_END)) {
            return;
        }
        //如果是注释则继续 
        if (!strcmp(buf, CFG_NOTES_FLAG)) {
            fscanf(fp, "%[^\n]%*c", buf);
            continue; 
        } 
        pYcMbREMap = new TY_YC_MB_REMAP();
        //未结束则buf中数据是遥信起始序号
        pYcMbREMap->ycModbusAddr = atoi(buf);//实际从1开始编号 
        fscanf(fp,"%s\n", &pYcMbREMap->formula);
        addYcREMap(&ycMbREMapLink, pYcMbREMap);		 
        printf("ModbufAddr:%d Formula:%s\n", pYcMbREMap->ycModbusAddr, &pYcMbREMap->formula);
    } 	
}

void addMbDevRegBlock(TY_REG_BLOCK* plink, TY_REG_BLOCK* pRegBlock)
{
    if (plink == NULL) {
        plink = pRegBlock;
    } else {
        while (plink->next != NULL) {
            plink = plink->next;
        }
        plink->next = pRegBlock;
    }
}
void addMbDev(TY_MBDEV_LINK* plink, TY_MB_DEVICE* pDev)
{
    pTY_MBDEV_LINK pnext;
    sprintf(pDev->tag, "%s:%d", pDev->ip, pDev->port); //生成标记
    if (plink->pdev == NULL){
        plink->pdev=pDev;
    } else {
        while (plink->next != NULL) {
            plink = plink->next;
        }
        pnext = new TY_MBDEV_LINK();
        pnext->pdev = pDev;
        plink->next = pnext; 
    }
}
void addYxMap(TY_YX_MB_MAP_LINK* plink, TY_YX_MB_MAP* pYxMap)
{
    pTY_YX_MB_MAP_LINK pnext; 
    if (plink->pYxMbMap == NULL) {
        plink->pYxMbMap = pYxMap;
    } else {
        while (plink->next != NULL) {
            plink = plink->next;
        }
            pnext = new TY_YX_MB_MAP_LINK();
            pnext->pYxMbMap = pYxMap;
            plink->next = pnext;	
    }
}
void addYcMap(TY_YC_MB_MAP_LINK* plink, TY_YC_MB_MAP* pYcMap)
{
    pTY_YC_MB_MAP_LINK pnext; 
    if (plink->pYcMbMap == NULL) {
        plink->pYcMbMap = pYcMap;
    } else {
        while (plink->next != NULL) {
            plink = plink->next;
        }
        pnext = new TY_YC_MB_MAP_LINK();
        pnext->pYcMbMap = pYcMap;
        plink->next = pnext;	
    }
}
void addYcREMap(TY_YC_MB_REMAP_LINK* plink, TY_YC_MB_REMAP* pYcREMap)
{
    pTY_YC_MB_REMAP_LINK pnext; 
    if (plink->pYcMbREMap == NULL) {
        plink->pYcMbREMap=pYcREMap;
    } else {
        while (plink->next != NULL) {
            plink = plink->next;
        }
        pnext = new TY_YC_MB_REMAP_LINK();
        pnext->pYcMbREMap = pYcREMap;
        plink->next = pnext;	
    }
}

void addMbTcpCli(char* host, ushort port)
{
    CModbusTcpCli* pMbTcpCli;
    pTY_MBTCPCLI_LINK plink, pnext;

    plink = &mbTcpCliLink;
    if (plink->pMbTcpCli == NULL) {
        pMbTcpCli = new CModbusTcpCli(host, port);//创建设备
        plink->pMbTcpCli = pMbTcpCli;
    } else {
        while (1) {
            pMbTcpCli = plink->pMbTcpCli;
            if (!strcmp(host, pMbTcpCli->m_host) && port == pMbTcpCli->m_port) {
                return;
            }
            if (plink->next == NULL){
                break;
            }
            plink = plink->next;
        }
        pnext = new TY_MBTCPCLI_LINK();
        pMbTcpCli = new CModbusTcpCli(host, port);//创建设备
        pnext->pMbTcpCli = pMbTcpCli;
        plink->next = pnext;
    }
    pMbTcpCli->open();//打开设备 
}
CModbusTcpCli* findMbTcpCliByTag(char* tag)
{
    pTY_MBTCPCLI_LINK plink = &mbTcpCliLink;
    while(plink!=NULL && plink->pMbTcpCli!=NULL) {
        if (!strcmp(plink->pMbTcpCli->tag, tag)) {
            return plink->pMbTcpCli;
        }
        plink = plink->next;
    }
    return NULL;
}
//打印mobbus TCP设备列表 
void printMbTcpCliList(void)
{
    pTY_MBTCPCLI_LINK plink = &mbTcpCliLink;
    while (plink != NULL) {
        printf("MbTcpCli:%s\n", plink->pMbTcpCli->tag);
        plink = plink->next; 
    }
}

//modbus扫描线程
DWORD WINAPI modbusScanThread(LPVOID lpThreadParameter)
{
    //建立tcp连接
    pTY_MBDEV_LINK pMbDevLink = &mbDevLink;
    while (pMbDevLink != NULL) {
        addMbTcpCli(pMbDevLink->pdev->ip, pMbDevLink->pdev->port);	
        pMbDevLink = pMbDevLink->next;
    } 
    printMbTcpCliList();	 
    
    //循环扫描 
    TY_MB_DEVICE* pdev;
    CModbusTcpCli* pMbTcpCli;
    TY_REG_BLOCK* pRegBlock;
    char *pregbuf;
    int ret;
    
    pMbDevLink = &mbDevLink;
    for (;;) {
        if (pMbDevLink==NULL) {
            pMbDevLink=&mbDevLink;
        }	
      
        pdev = pMbDevLink->pdev;
        pMbTcpCli = findMbTcpCliByTag(pdev->tag);
        printf("Scan pMbTcpCli:%s\n", pMbTcpCli->tag);
        if (pMbTcpCli == NULL) {
            printf("not found pMbTcpCli:%s\n", pdev->tag);
            continue;
        }
      
        pRegBlock = pdev->pRegBlock;      
        while (pRegBlock != NULL) {
            pregbuf = (char*)(pdev->regVal + pRegBlock->s_add); 
            printf("reg read mbaddr:%d,func:%d,s_addr:%d,size:%d,%s\n",
                    pdev->mbAddr, pRegBlock->func, pRegBlock->s_add, pRegBlock->size,pdev->tag);
            if ((ret = pMbTcpCli->readReg(pdev->mbAddr, pRegBlock->func, pRegBlock->s_add, pRegBlock->size, pregbuf)) != 0) {
                if (ret == -1) {
                    //通信链路错误
                    pMbTcpCli->commonErrorCnt++;
                    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
                    SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
                    printf("tcp link error %s\n", pdev->tag);
                    printf("reg read mbaddr:%d,func:%d,s_addr:%d,size:%d,%s\n",
                            pdev->mbAddr, pRegBlock->func, pRegBlock->s_add, pRegBlock->size,pdev->tag);	
                    SetConsoleTextAttribute(handle, 0x07);	      
                } else {
                     //读取失败
                    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);	
                    SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
                    printf("reg read error mbaddr:%d,func:%d,s_addr:%d,size:%d,%s\n",
                            pdev->mbAddr, pRegBlock->func, pRegBlock->s_add, pRegBlock->size, pdev->tag);	
                    SetConsoleTextAttribute(handle, 0x07);
                }   	          
            }
            //超过最大通信错误次数后重连 
            if (pMbTcpCli->commonErrorCnt >= MAX_COMMON_ERROR_CNT) {
                pMbTcpCli->close();
                pMbTcpCli->open();
            }
            pRegBlock = pRegBlock->next;      
             //扫描间隔 
            Sleep(serailParam.devScanInterval);	  
        } 
        pMbDevLink = pMbDevLink->next;  
    }
} 
void startModbusScan(void)
{
    DWORD lpThreadId = 0;
    HANDLE h2 = CreateThread(NULL, 0, modbusScanThread, NULL, 0, &lpThreadId); 
}

//根据设备地址和寄存器地址获取寄存器值 
int getMbRegValue(uchar mbaddr, ushort s_regaddr, int size, ushort byteorder, ushort* buf, char* tag)
{
    pTY_MBDEV_LINK pMbDevLink = &mbDevLink;
    TY_MB_DEVICE* pdev;
    while (pMbDevLink != NULL) {
        if (pMbDevLink->pdev->mbAddr == mbaddr && !strcmp(pMbDevLink->pdev->tag, tag)) {
            pdev = pMbDevLink->pdev;
            //此处加锁 
            EnterCriticalSection(&CriticalSection);
            memcpy(buf, pdev->regVal+s_regaddr, size*2);
            //此处解锁 
            LeaveCriticalSection(&CriticalSection); 
            if (byteorder == 0) {//字节顺序 不做处理 
                switchBytes((uchar*)buf, size*2); 
            } else if (byteorder == 0x01) {//交换高低字节 
                return 0;
            } else if (byteorder == 0x02) {//交换高低寄存器 
                switchBytes((uchar*)buf, size*2);
                switchReg(buf, size);
            } else if(byteorder == 0x04) {//倒叙位 
                switchBytes((uchar*)buf, size*2);
                reverseBit((uchar*)buf, size*2);
            }		
            return 0;
        }
        pMbDevLink = pMbDevLink->next;
    }
    return -1;
}

//发送数据到串口 
void send2serail(uchar mbDevAddr, uchar mbFunc, ushort mbBytes, uchar *pval, uint Serail_NUM)
{
    uchar sbuf[255];
    memset(sbuf, 0, sizeof(sbuf));
    ushort u16crcdata;
    uchar u8crcdata[3];
    sbuf[0] = mbDevAddr;//设备地址 
    sbuf[1] = mbFunc;//功能码 
    sbuf[2] = mbBytes&0xff;//字节数 寄存器数*2 
    memcpy(sbuf+3, pval, mbBytes); //添加数据 
    u16crcdata = CRC16(sbuf, 3+mbBytes);
    u8crcdata[0] = u16crcdata>>8;
    u8crcdata[1] = u16crcdata&0xff;
    u8crcdata[2] = '\0';
    memcpy(sbuf+mbBytes+3, u8crcdata, 3);//添加校验 
    io_write((char*)sbuf, 5+mbBytes, Serail_NUM);
}

//转换寄存器值
void convertMbReg(uchar *sbuf, uint Serail_NUM)
{
    ushort *pbuf;
    float val;
    pTY_YX_MB_MAP_LINK pyxlink = &yxMbMapLink;
    pTY_YC_MB_REMAP_LINK pyclink = &ycMbREMapLink;
    TY_YX_MB_MAP* pYxMbMap;
    TY_YC_MB_REMAP* pYcMbREMap;
    
    uchar buf[255];
    pbuf = (ushort*)buf;
    memset(buf, 0, sizeof(buf));
    uchar mbDevAddr = sbuf[0];//设备地址
    uchar mbFunc = sbuf[1];//功能码	
    ushort mbRegStart = sbuf[3]+(sbuf[2]<<8);//寄存器地址
    ushort mbRegSize = sbuf[5]+(sbuf[4]<<8);//寄存器个数 	
    ushort mbRegSurplus = mbRegSize;//剩余未读寄存器个数 	
     
    //读取遥信值
    if (mbRegStart < CFG_YC_START_SEQ) {		
        for (; pyxlink != NULL; pyxlink=pyxlink->next) {			
            pYxMbMap = pyxlink->pYxMbMap;
            if (mbDevAddr==MachineAddr && (mbRegStart+mbRegSize-mbRegSurplus)==pYxMbMap->yxSeqStart) {
                if (getMbRegValue(pYxMbMap->mbDevAddr, pYxMbMap->mbRegStart, pYxMbMap->mbRegSize, 1, pbuf, pYxMbMap->tag) != 0) {
                    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);	
                    SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
                    printf("getMbRegValue faile mbDevAddr:%d,mbRegStart:%d,mbRegSize:%d\n",
                            pYxMbMap->mbDevAddr,pYxMbMap->mbRegStart,pYxMbMap->mbRegSize);  	
                    SetConsoleTextAttribute(handle, 0x07);	
                    continue;			
                }
                if (pYxMbMap->bit_start != 0xFF) {//遥信使用位
                    *pbuf = ((*pbuf)&(1<<pYxMbMap->bit_start))>0?1:0;
                    switchBytes((uchar*)pbuf, pYxMbMap->mbRegSize*2); 
                } else if (pYxMbMap->bit_start == 0xFF) {//不使用位 
                    *pbuf = (*pbuf)>0?1:0;
                }
                mbRegSurplus -= pYxMbMap->mbRegSize;
                if (mbRegSurplus <= 0) {
                    break;	
                }
                pbuf += pYxMbMap->mbRegSize;//偏移 					
            }
        }
    }
    
    //读取遥测值
    if (mbRegStart >= CFG_YC_START_SEQ) {
        for (; pyclink != NULL; pyclink=pyclink->next) {			
            pYcMbREMap = pyclink->pYcMbREMap;
            if (mbDevAddr==MachineAddr && (mbRegStart+mbRegSize-mbRegSurplus)==pYcMbREMap->ycModbusAddr) {
                val = getCalcValue(pYcMbREMap->formula, ycMbMapLink);
                memcpy(pbuf, &val, sizeof(val));
                switchBytes((uchar*)pbuf, sizeof(val));
                printf("mbaddr:%d val:%.3f\n", pYcMbREMap->ycModbusAddr, val);
                mbRegSurplus -= (sizeof(val)/2);
                if (mbRegSurplus <= 0) {
                    break;
                }	
                pbuf += (sizeof(val)/2);//偏移 	
            }
        }	
    }
    printf("mbDevAddr:%d,mbRegStart:%d,mbRegSize:%d,Serail:COM%d\n",mbDevAddr, mbRegStart, mbRegSize, Serail_NUM);
    send2serail(mbDevAddr, mbFunc, mbRegSize*2, buf, Serail_NUM);
}

//串口Modbus线程 
void startModbusSerail(void)
{
    uchar sbuf[255];
    for (;;) {		
        for (uint i=0; i<Serail_count; i++) {//循环访问已打开的串口 
            memset(sbuf, 0, sizeof(sbuf));
            serail.read((char*)sbuf, 8, Serail_NUM[i]);
            if (CheckSerailMsg(sbuf) == 1) {
                convertMbReg(sbuf,Serail_NUM[i]);
            }
            Sleep(serailParam.simFlushT);
        } 		
    } 
}


/******************************************* 原模拟屏程序 ***************************************************/
/*
//设置单个遥信量 
void setOneYx(uchar* yxs, int yxSeq, uchar yxStatus){	
    int index,bitpos;	
    uchar* p;
    
    printf("setOneYx yxSeq:%d yxStatus:%d\n",yxSeq, yxStatus);
    
    yxSeq-=1;
    index = yxSeq/4;
    bitpos = (yxSeq%4)*2;	
    p = yxs + index;	
    
    *p &= ~(0x03<<bitpos);//清零指定位 
    *p |= (yxStatus<<bitpos);//设置指定位 
} 
uchar convert2YxStatusBybit(ushort val, int bit){
    return ((val&(1<<bit))>0?YX_RED:YX_GREEN);
}
uchar convert2YxStatus(ushort val){
    return (val>0?YX_RED:YX_GREEN);
}
//将寄存器值转换成遥信量 
void convertMbReg2Yx(uchar* yxs){
    ushort i,val,*pval;
    uchar buf[255];
    pTY_YX_MB_MAP_LINK plink=&yxMbMapLink; 
    TY_YX_MB_MAP* pYxMbMap;
    
    
    for(i=0; plink != NULL; plink=plink->next){
        pval=(ushort*)buf;
        pYxMbMap=plink->pYxMbMap;
        if(getMbRegValue(pYxMbMap->mbDevAddr,pYxMbMap->mbRegStart,pYxMbMap->mbRegSize,pval)!=0){
            printf("getMbRegValue faile mbDevAddr:%d,mbRegStart:%d,mbRegSize:%d\n",
                     pYxMbMap->mbDevAddr,pYxMbMap->mbRegStart,pYxMbMap->mbRegSize);
            continue;
        }
        printf("mbaddr:%d ",pYxMbMap->mbDevAddr);
        //不使用位 
        if(pYxMbMap->bit_start==0xFF){
           for(i=0;i<pYxMbMap->yxSize;i++){
                    setOneYx(yxs, pYxMbMap->yxSeqStart+i, convert2YxStatus(*pval));
                    pval++;
              }	
        }
        //使用位 
        else{
               setOneYx(yxs, pYxMbMap->yxSeqStart, convert2YxStatusBybit(*pval, pYxMbMap->bit_start));
        } 
    } 
}
//yx CDT广播线程
void startYxCdtBroadCast(){
    //送所有YX数据 
    uchar yxs[30];
    memset(yxs,0xAA,sizeof(yxs));
    for(;;){
        convertMbReg2Yx(yxs);
        sendBatchYX(1,yxs,120);
        Sleep(serailParam.simFlushT);
    } 
}
*/

