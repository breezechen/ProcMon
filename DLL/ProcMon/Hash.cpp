#include <windows.h>
#include "log.h"
#include "Hash.h"
#include "psapi.h"
#pragma comment(lib,"Psapi")

//global
ULONG crc_tab[256];

ULONG getHash(const DWORD id)
{
	PIMAGE_OPTIONAL_HEADER optionalHeader;
	PIMAGE_DOS_HEADER dosHeader;
	WCHAR lpFileName[MAX_PATH];
	DWORD simCount = sizeof(lpFileName);
	PVOID Buffer;
	HANDLE exeObj;
	HANDLE pid;
	ULONG hash;

	InitHashTable();
	pid = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE , id);
	if(!pid)
	{
		log("Cant Open Process(getHash)\n");
		CloseHandle(pid);
		CloseHandle(exeObj);
		return 0;
	}
	if(!GetModuleFileNameEx(pid, NULL, lpFileName, MAX_PATH))
	{
		log("Cant get file path(getHash)\n");
		CloseHandle(pid);
		CloseHandle(exeObj);
		return 0;
	}
	CloseHandle(pid);
	exeObj = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
	if(exeObj == INVALID_HANDLE_VALUE)
	{
		log("Cant create file(getHash)\n");
		CloseHandle(exeObj);
		return 0;
	}
	Buffer = malloc(1024);
	ReadFile(exeObj, Buffer, 1024, &simCount, NULL);
	CloseHandle(exeObj);
	dosHeader = (PIMAGE_DOS_HEADER)Buffer;
	optionalHeader = (PIMAGE_OPTIONAL_HEADER)((UCHAR *)Buffer + dosHeader->e_lfanew + 24);
	free(Buffer);
	if (optionalHeader == NULL)
	{
		log("Cant get OptionalHeader(getHash)\n");
		return 0;
	}
	hash = chksum_crc32 ((UCHAR *) optionalHeader, sizeof(IMAGE_OPTIONAL_HEADER));
	return hash;
}

ULONG chksum_crc32 (UCHAR *block, USHORT length)
{
   ULONG crc, i;

   crc = 0xFFFFFFFF;
   for (i = 0; i < length; i++)
   {
      crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
   }
   return (crc ^ 0xFFFFFFFF);
}

void InitHashTable ()
{
   ULONG crc, poly;
   SHORT i, j;

   poly = 0xEDB88320L;
   for (i = 0; i < 256; i++)
   {
      crc = i;
      for (j = 8; j > 0; j--)
      {
	 if (crc & 1)
	 {
	    crc = (crc >> 1) ^ poly;
	 }
	 else
	 {
	    crc >>= 1;
	 }
      }
      crc_tab[i] = crc;
   }
}