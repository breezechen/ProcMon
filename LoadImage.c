#include <ntifs.h>

typedef struct {
  PWCHAR fullName;
  PWCHAR name;
  PWCHAR hash;
  SINGLE_LIST_ENTRY SingleListEntry;
  ULONG pId;
} BLIST_ENTRY, *PBLIST_ENTRY;

void
PushBLISTEntry(PSINGLE_LIST_ENTRY ListHead, PBLIST_ENTRY Entry)
{
    PushEntryList(ListHead, &(Entry->SingleListEntry));
}

PBLIST_ENTRY
PopBLISTEntry(PSINGLE_LIST_ENTRY ListHead)
{
    PSINGLE_LIST_ENTRY SingleListEntry;
    SingleListEntry = PopEntryList(ListHead);
    return CONTAINING_RECORD(SingleListEntry, BLIST_ENTRY, SingleListEntry);
}


static VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	HANDLE hProcess = NULL;
	NTSTATUS status;
	PEPROCESS peProcess;


	PWCHAR path;
	USHORT length,i;
	UNICODE_STRING ext;
	UNICODE_STRING exeExt;
	UNICODE_STRING ProcName;
	UNICODE_STRING notepad;

	// this is test stab
	RtlInitUnicodeString(&notepad, L"notepad.exe");

	// Init String with "exe"
	RtlInitUnicodeString(&exeExt, L"exe");

	length = (FullImageName->Length)/2;
	path = FullImageName->Buffer;

	// Init string with extension of file
	RtlInitUnicodeString(&ext,(FullImageName->Buffer+(length-3)));

	// Compare extensions
	if ( RtlCompareUnicodeString(&ext,&exeExt,TRUE) == 0 )
	{
	i = length - 1 ;
	{
		// Find point to "\" in path
		while (i>0)
		{
			if (path[i] == '\\')
				break;
			i--;
		}
		RtlInitUnicodeString(&ProcName,(FullImageName->Buffer+(++i)));

		if ( RtlCompareUnicodeString(&ProcName,&notepad,TRUE) == 0 )
		{
			DbgPrint("Process ID: %d",ProcessId);
			DbgPrint("Process Name: %wZ",&ProcName);
			status = PsLookupProcessByProcessId(ProcessId, &peProcess);
			if (status != STATUS_SUCCESS)
				DbgPrint(("Err PsLookupProcessByProcessId\n"));
			DbgPrint("Process ID: %d",ProcessId);
		
			status = ObOpenObjectByPointer(peProcess, OBJ_KERNEL_HANDLE,
										NULL, DELETE, NULL, KernelMode,
										&hProcess);
			if(status != STATUS_SUCCESS)
				DbgPrint("ObOpenObjectByPointer error");
		
			status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
			if(status)
				DbgPrint("ZwTerminateProcess error /n");
		}
	}
	}

}


VOID UnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
    PsRemoveLoadImageNotifyRoutine(  LoadImageNotifyRoutine );
    DbgPrint("Unload!\n");
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,  IN PUNICODE_STRING RegistryPath)
{
    PsSetLoadImageNotifyRoutine( LoadImageNotifyRoutine );
    DriverObject->DriverUnload = UnloadRoutine;
    DbgPrint("Driver loaded");
    
    return STATUS_SUCCESS; 

}