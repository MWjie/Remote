#ifndef NET_H
#define NET_H

#include <iostream>
#include <windows.h> 
#include <winsock2.h>
#include "type.h"

using namespace std;

class CTcpClient {
    private:
        SOCKET m_sock;   //socket¨¬¡Á?¨®¡Á?
    public:
        char*  m_host;
        ushort m_port;
    public:
        int open(char* host, ushort port);
        int close(void);
        int read(char* buf, int size);
        int write(char* buf, int size);
};

#endif
