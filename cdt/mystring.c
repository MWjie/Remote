#include "mystring.h"

int m_find(char* str, char flag, int time)
{
    int pos = 0, _time = 1;
    while (*str != '\0') {
        if (*str == flag) {
            if (time == _time) {
                return pos;				
            }
            _time++;
        }
        pos++;
        str++;
    }
}
int m_substr(char* str, char* dest, char flag)
{
    int pos = 1;
    while (1) {
        if (*str == flag) {
            return pos;
        }
        if (*str == '\0') {
            return 0;
        }
        
        *dest = *str;
        pos++;
        str++;
        dest++;
    }
    return -1;
}
