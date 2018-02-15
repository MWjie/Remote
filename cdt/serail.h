#ifndef CSERAIL_H
#define CSERAIL_H

#include <iostream>
#include <windows.h> 

#define MAX_SERAIL_NUM 20//定义最大串口号
 
using namespace std;

class CSerail {
    private:
        HANDLE m_handle[MAX_SERAIL_NUM];
    public:
        int open(char* dev, int baud, int databit, int check, int stopbit, int selail_number);
        int close(int selail_number);
        int read(char* buf, int size, int selail_number);
        int write(char* buf, int size, int selail_number);
        int settimeout(DWORD ReadIntervalTimeout, 
            DWORD ReadTotalTimeoutMultiplier,
                DWORD ReadTotalTimeoutConstant,
                    DWORD WriteTotalTimeoutMultiplier, 
                        DWORD WriteTotalTimeoutConstant,
                            int selail_number);
};

#endif 
