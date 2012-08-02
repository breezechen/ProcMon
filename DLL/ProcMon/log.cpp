#include <windows.h>
#include <time.h>
#include <iostream>
#include "log.h"

void log(char* str)
{
	SYSTEMTIME sm;
	char* buf = new char[16];

	GetLocalTime(&sm);
	FILE* file = fopen("dllLog.txt", "a");
	buf = itoa(sm.wYear, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite(".", 1, 1, file);
	itoa(sm.wMonth, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite(".", 1, 1, file);
	itoa(sm.wDay, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite("  ", 2, 1, file);
	itoa(sm.wHour, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite(":", 1, 1, file);
	itoa(sm.wMinute, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite(":", 1, 1, file);
	itoa(sm.wSecond, buf, 10);
	fwrite(buf, strlen(buf), 1, file);
	fwrite(" :  ", 4, 1, file);
	fwrite(str,strlen(str), 1, file);
	fclose(file);
	delete[] buf;
}