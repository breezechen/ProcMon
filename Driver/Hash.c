#include <ntifs.h>
#include "Hash.h"

ULONG crc_tab[256];
ULONG hash = 0;

BOOLEAN GetFullPath(IN PUNICODE_STRING NtFullName);

ULONG GetHashCode(IN PUNICODE_STRING NtFullName);

VOID InitHashTable ();

ULONG chksum_crc32 (UCHAR *block, USHORT length);


BOOLEAN GetFullPath(IN PUNICODE_STRING NtFullName)
{
	HANDLE fileHandle;
	OBJECT_ATTRIBUTES obAttribute; 
	IO_STATUS_BLOCK statusBlock; 
	NTSTATUS status ;
	PFILE_OBJECT fObject = NULL;
	POBJECT_NAME_INFORMATION obNameInfo;


	InitializeObjectAttributes(&obAttribute,NtFullName,OBJ_KERNEL_HANDLE,NULL,NULL);
	status = ZwOpenFile(&fileHandle,SYNCHRONIZE|GENERIC_READ, &obAttribute , &statusBlock , FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE , FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT ) ;
	if (!NT_SUCCESS(status))
	{
		#if DBG
		DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "ZwOpenFile", status);
		#endif
		return FALSE;
	}

	status = ObReferenceObjectByHandle(fileHandle, NULL, *IoFileObjectType, KernelMode, &fObject, NULL);
	if (!NT_SUCCESS(status))
	{
		#if DBG
		DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "ObReferenceObjectByHandle", status);
		#endif
		ZwClose(fileHandle);
		return FALSE;
	}
	IoQueryFileDosDeviceName(fObject,&obNameInfo);
	#if DBG
	DbgPrint("Path: %wZ\n",&obNameInfo->Name);
	#endif

	ObDereferenceObject(fObject);
	ZwClose(fileHandle);

	return TRUE;
}

ULONG GetHashCode(IN PUNICODE_STRING NtFullName)
{
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_OPTIONAL_HEADER optionalHeader;
	PVOID Buffer;
	HANDLE fileHandle;
	OBJECT_ATTRIBUTES obAttribute; 
	IO_STATUS_BLOCK statusBlock; 
	NTSTATUS status ;
	USHORT Magic;

	InitializeObjectAttributes(&obAttribute,NtFullName,OBJ_KERNEL_HANDLE,NULL,NULL);
	status = ZwOpenFile(&fileHandle,SYNCHRONIZE|GENERIC_READ, &obAttribute , &statusBlock , FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE , FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT ) ;
	if (!NT_SUCCESS(status))
	{
		#if DBG
		DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "ZwOpenFile", status);
		#endif
		return 0;
	}
	Buffer = ExAllocatePool(PagedPool,1024);
	ZwReadFile(fileHandle,NULL,NULL,NULL,&statusBlock,Buffer,1024,NULL,NULL);
	
	dosHeader = (PIMAGE_DOS_HEADER)Buffer;
	optionalHeader = (PIMAGE_OPTIONAL_HEADER)((UCHAR *)Buffer + dosHeader->e_lfanew + 24);

	if (optionalHeader == NULL)
	{
		#if DBG
		DbgPrint("Unable to get address of optional header block");
		#endif
		ZwClose(fileHandle);
		ExFreePool(Buffer);
		return 0;
	}

	hash = chksum_crc32 ((UCHAR *) optionalHeader, sizeof(IMAGE_OPTIONAL_HEADER));
	#if DBG
	DbgPrint("Hash: %p\n", hash);
	#endif
	ExFreePool(Buffer);
	return hash;
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