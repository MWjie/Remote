#include "serail.h"

int CSerail::open(char* dev, int baud, int databit, int check, int stopbit, int selail_number) 
{
    m_handle[selail_number] = CreateFile(dev,
        GENERIC_READ | GENERIC_WRITE,
        0,//独占 
        NULL,
        OPEN_EXISTING, // 打开而不是创建
        FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_OVERLAPPED*/, // 非重叠方式
        NULL
    );
    
    if (m_handle[selail_number] == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError(); // 处理错误
        return 1; 
    }

    //设置串口参数 
    DCB dcb;
    GetCommState(m_handle[selail_number], &dcb); 
    
    dcb.BaudRate = (DWORD)baud;
    dcb.ByteSize = (BYTE)databit;
    dcb.Parity   = (BYTE)check;//NOPARITY,EVENPARITY
    dcb.StopBits = ONESTOPBIT;//ONESTOPBIT
    
    SetCommState(m_handle[selail_number], &dcb);    

    return 0;
}

int CSerail::close(int selail_number)
{
    if (m_handle[selail_number] != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle[selail_number]);		
    }
    return 0;	
}

int CSerail::read(char* buf, int size, int selail_number)
{
    DWORD realsize = 0; 
    
    if (m_handle[selail_number] == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    if (ReadFile(
        m_handle[selail_number],      // 指向标识的句柄
        buf,    // 指向一个缓冲区
        size, // 读取的字节数
        &realsize, // 指向调用该函数读出的字节数
        NULL  // 一个OVERLAPPED的结构
    )){
        return realsize;
    }
    
    return 0;		
}

int CSerail::write(char* buf, int size, int selail_number)
{
    DWORD realsize = 0; 

    if (m_handle[selail_number] == INVALID_HANDLE_VALUE) {
       return -1;
    }
    
    if (WriteFile(
        m_handle[selail_number],      // 指向标识的句柄
        buf,    // 指向一个缓冲区
        size, // 指定要向串口设备写入的字节数
        &realsize, // 指向调用该函数已写入的字节数
        NULL   // 一个OVERLAPPED的结构
    )){
        return realsize;
    }
    
    return 0;				
}

int CSerail::settimeout(DWORD ReadIntervalTimeout, 
    DWORD ReadTotalTimeoutMultiplier,
        DWORD ReadTotalTimeoutConstant,
            DWORD WriteTotalTimeoutMultiplier, 
                DWORD WriteTotalTimeoutConstant,
                    int selail_number)
{
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(m_handle[selail_number], &timeouts);
    
    /*以ms为单位指定通信线路上两个字符到达之间的最大时间间隔。
    在ReadFile()操作期间，从接收到第一个字符时开始计时。
    如果任意两个字符到达之间的时间间隔超过这个最大值，
    则ReadFile()操作完成，并返回缓冲数据。如果被置为0，则表示不使用间隔超时。*/
    timeouts.ReadIntervalTimeout = ReadIntervalTimeout;
    timeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;//以ms为单位指定一个系数，该系数用来计算读操作的总超时时间。
    timeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;//以ms为单位指定一个常数，该常数也用来计算读操作的总超时时间。
    timeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier;//以ms为单位指定一个系数，该系数用来计算写操作的总超时时间。
    timeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant;//以ms为单位指定一个常数，该常数也用来计算写操作的总超时时间。
    
    //ReadTotalTimeout=( ReadTotalTimeoutMultiplier*bytes_to_read )+ ReadTotalTimeoutConstant;
    //WriteTotalTimeout=( WriteTotalTimeoutMultiplier*bytes_to_write )+ WriteTotalTimeoutConstant;
    
    SetCommTimeouts(m_handle[selail_number], &timeouts);
}
