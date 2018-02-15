#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "calculate.h"
#include "simscreen.h"
#include "ModbusTCPCli.h"

#define MAX_SIZE 100 //���ջ��

//��������ȼ�
enum PriOperator {
    PriERR,
    PriOne,
    PriTwo
};

//ջ״̬
enum usu {
    STACK_FULL = -10,
    STACK_OK,
    STACK_EMPTY,
    SUCCESS,
    PUSH_SUCCESS
};

float calculate(float num1, float num2, char operat)
{
    switch (operat) {
        case '+': return (num1 + num2);
        case '-': return (num1 - num2);
        case '*': return (num1 * num2);
        case '/': return (num1 / num2);
    }
    return 0;
}

int priority(char operat)
{
    switch (operat) {
        case '+':
        case '-': return PriOne;
        case '*':
        case '/': return PriTwo;
        default: return PriERR;
    }
    return PriERR;
}

int is_StackFull(int *top)
{
    if (*top == MAX_SIZE-1) {
        return STACK_FULL;
    }
    return STACK_OK;
}

int is_StackEmpty(int *top)
{
    if (*top == -1) {
        return STACK_EMPTY;
    }
    return STACK_OK;
}

int push_StackNum(float *stack, float *num, int *top)
{
    if (is_StackFull(top) == STACK_FULL) {
        return STACK_FULL;
    }
    (*top)++;
    stack[*top] = *num;
    return PUSH_SUCCESS;
}

int push_StackOperator(char *stack, char *operat, int *top)
{
    if (is_StackFull(top) == STACK_FULL) {
        return STACK_FULL;
    }
    (*top)++;
    stack[*top] = *operat;
    return PUSH_SUCCESS;
}

float pop_StackNum(float *stack, int *top)
{
    float num;
    if (is_StackFull(top) == STACK_EMPTY) {
        return STACK_EMPTY;
    }
    num = stack[*top];
    (*top)--;
    return num;
}

char pop_StackOperator(char *stack, int *top)
{
    char operat;
    if (is_StackFull(top) == STACK_EMPTY) {
        return STACK_EMPTY;
    }
    operat = stack[*top];
    (*top)--;
    return operat;
}

int compare_priority(char *str, char *stack_operator, int *top_operator, 
                                    float *stack_num, int *top_num)
{
    char operat;
    float num1, num2, total;
    
    if (*str == '\0'){//����ʽ����
        return SUCCESS;
    }
    if (is_StackFull(top_operator) == STACK_FULL){
        return STACK_FULL;
    }
    else if (is_StackEmpty(top_operator) == STACK_EMPTY || *str == '(') {
        push_StackOperator(stack_operator, str, top_operator);
    }
    else if (*str == ')') {
        operat = pop_StackOperator(stack_operator, top_operator);//��ջ��ȡ�������
        while (operat != '(') {
            num1 = pop_StackNum(stack_num, top_num);//ȡ��������
            num2 = pop_StackNum(stack_num, top_num);
            total = calculate(num2, num1, operat);//����
            push_StackNum(stack_num, &total, top_num);//�����ջ
            operat = pop_StackOperator(stack_operator, top_operator);
        }
    }
    else {//����+-*/
        operat = pop_StackOperator(stack_operator, top_operator);
        if (operat == '(') {
            push_StackOperator(stack_operator, &operat, top_operator);
            push_StackOperator(stack_operator, str, top_operator);
            return SUCCESS;
        }
        if (priority(operat) > priority(*str)) {//��������ȼ� 
            push_StackOperator(stack_operator, &operat, top_operator);
            push_StackOperator(stack_operator, str, top_operator);
        }
        else {
            num1 = pop_StackNum(stack_num, top_num);//ȡ��������
            num2 = pop_StackNum(stack_num, top_num);
            total = calculate(num2, num1, operat);//����
            push_StackNum(stack_num, &total, top_num);//�����ջ
            compare_priority(str, stack_operator, top_operator, stack_num, top_num);
        }
    }
    return SUCCESS;
}

float getCalcValue(char *str, TY_YC_MB_MAP_LINK ycMbMapLink)
{
    float stack_num[MAX_SIZE];//������ջ
    char stack_operator[MAX_SIZE];//�����ջ
    memset(stack_num, 0, sizeof(stack_num));
    memset(stack_operator, 0, sizeof(stack_operator));
    int top_operator = -1, top_num = -1;

    float num = 0, num1 = 0, num2 = 0, total = 0;
    char operat = 0;
    char *ptr = str;
    
    while (*ptr != '\0') {
        //��������ջ
        if ((*ptr>='0' && *ptr<='9') || *ptr == '.') {
            num = atof(ptr);
            while ((*ptr>='0' && *ptr<='9') || *ptr == '.') {
                ptr++;
            }
            push_StackNum(stack_num, &num, &top_num);
        }
        if (*ptr == '[') {
            ushort *pbuf;
            uchar buf[64];
            pbuf = (ushort*)buf;
            memset(buf, 0, sizeof(buf));

            pTY_YC_MB_MAP_LINK pyclink = &ycMbMapLink;
            TY_YC_MB_MAP* pYcMbMap;

            float fnum = 0;//�����������ݴ��� 
            short snum = 0;//�����������ݴ��� 
            ushort seq = atoi(++ptr);//�ж����
            while (1) {
                if (*ptr == ']') {
                    ptr++;
                    break;
                }
                ptr++;
            }
        
            for (; pyclink != NULL; pyclink=pyclink->next) {			
                pYcMbMap = pyclink->pYcMbMap;
                if (pYcMbMap->ycSeqStart == seq) {
                    if (getMbRegValue(pYcMbMap->mbDevAddr, pYcMbMap->mbRegStart, pYcMbMap->mbRegSize, pYcMbMap->byteorder, pbuf, pYcMbMap->tag) != 0) {
                        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
                        SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
                        printf("getMbRegValue faile mbDevAddr:%d,mbRegStart:%d,mbRegSize:%d\n",
                                pYcMbMap->mbDevAddr, pYcMbMap->mbRegStart, pYcMbMap->mbRegSize);
                        SetConsoleTextAttribute(handle, 0x07);				
                    }
                    if (pYcMbMap->dataType == 1) {//�������� 
                        switchBytes(buf, sizeof(snum));
                        memcpy(&snum, pbuf, sizeof(snum));
                        fnum = (float)snum;//������ǿתΪ������ڴ��� 
                    } else if (pYcMbMap->dataType == 2) {//�������� 
                        switchBytes(buf, sizeof(fnum));
                        memcpy(&fnum, pbuf, sizeof(fnum));
                    } else {
                        printf("\nSeq:%d Reg:%d Addr:%d datatype error!\n",pYcMbMap->ycSeqStart, pYcMbMap->mbRegStart, pYcMbMap->mbDevAddr);
                    }	
                    push_StackNum(stack_num, &fnum, &top_num); 
                    break;
                }
            }
        }
        //�������ջ
        compare_priority(ptr, stack_operator, &top_operator, stack_num, &top_num);
        if (*ptr++ == '\0') {
            break;
        }
    }
    while (top_operator != -1) {//���� 
        num1 = pop_StackNum(stack_num, &top_num);
        num2 = pop_StackNum(stack_num, &top_num);
        operat = pop_StackOperator(stack_operator, &top_operator);
        total = calculate(num2, num1, operat);
        push_StackNum(stack_num, &total, &top_num);
    }
    if (num1==0 && num2==0 && num==0 && operat==0) {
        total = pop_StackNum(stack_num, &top_num);
    }
    return total;
}