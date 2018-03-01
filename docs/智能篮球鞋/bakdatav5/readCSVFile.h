#pragma once
#include <stdio.h>
#include <memory.h> // for memset
#include <stdlib.h> // for malloc, free
#include <string.h> // for strtok


#define MAX_LINE_SIZE 256   //fgets函数读取的最大字节数


float* giCsvData;           //动态分配数据内存的指针
int giNumData;            //读取的数据字节数
int giNumRow;             //每行的字节数
int giNumCol;             // 每列的字节数


int GetTotalLineCount(FILE* fp);              //计算csv文件中的总行数
int GetTotalColCount(FILE * fp);              //计算csv文件中的总列数（以第一行的列数为基准）
int AssignSpaceForData(int inumdata);         //通过指针*giCsvData给数据动态分配内存空间
void FreeCsvData();                           //释放动态数据内存
int ReadCsvData(char* csvFilePath);           //读取Csv中数据（本例默认数据类型为int）
void  ShowCsvData();                           //通过控制台显示读取的csv数据