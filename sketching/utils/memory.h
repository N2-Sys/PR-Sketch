#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct MEM_PACKED         //定义一个mem occupy的结构体
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned long total;
	char name2[20];
}MEM_OCCUPY;
 
 
typedef struct MEM_PACK         //定义一个mem occupy的结构体
{
	double total;
	double used_rate;
}MEM_PACK;

MEM_PACK* get_memoccupy();
