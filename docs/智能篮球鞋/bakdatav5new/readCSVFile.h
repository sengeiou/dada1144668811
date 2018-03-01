#pragma once
#include <stdio.h>
#include <memory.h> // for memset
#include <stdlib.h> // for malloc, free
#include <string.h> // for strtok


#define MAX_LINE_SIZE 256   //fgets������ȡ������ֽ���


float* giCsvData;           //��̬���������ڴ��ָ��
int giNumData;            //��ȡ�������ֽ���
int giNumRow;             //ÿ�е��ֽ���
int giNumCol;             // ÿ�е��ֽ���


int GetTotalLineCount(FILE* fp);              //����csv�ļ��е�������
int GetTotalColCount(FILE * fp);              //����csv�ļ��е����������Ե�һ�е�����Ϊ��׼��
int AssignSpaceForData(int inumdata);         //ͨ��ָ��*giCsvData�����ݶ�̬�����ڴ�ռ�
void FreeCsvData();                           //�ͷŶ�̬�����ڴ�
int ReadCsvData(char* csvFilePath);           //��ȡCsv�����ݣ�����Ĭ����������Ϊint��
void  ShowCsvData();                           //ͨ������̨��ʾ��ȡ��csv����