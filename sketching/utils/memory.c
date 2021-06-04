#include "./memory.h"

MEM_PACK *get_memoccupy ()    // get RAM message
{
	FILE *fd;
	char buff[256];
	MEM_OCCUPY *m=(MEM_OCCUPY *)malloc(sizeof(MEM_OCCUPY));
	MEM_PACK *p=(MEM_PACK *)malloc(sizeof(MEM_PACK));
	fd = fopen ("/proc/meminfo", "r");

	fgets (buff, sizeof(buff), fd);
	sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
	p->total = m->total / (1024*1024);

	fgets (buff, sizeof(buff), fd);
	sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
	p->used_rate = (1 - m->total/m->total)*100;

	fclose(fd);     //关闭文件fd
	return p;
}

