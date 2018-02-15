#ifndef MYSTRING_H
#define MYSTRING_H

#include "type.h"

#ifdef __cplusplus  
extern "C" {  
#endif 

int m_find(char* str, char flag, int time);
int m_substr(char* str, char* dest, char flag);

#ifdef __cplusplus  
}  
#endif

#endif
