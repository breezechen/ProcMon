#include <ntddk.h>

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

NTSTATUS PsLookupProcessByProcessId(__in   HANDLE ProcessId,
									__out  PEPROCESS *Process);

static VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

NTSTATUS ObOpenObjectByPointer(__in      PVOID Object,
							   __in      ULONG HandleAttributes,
							   __in_opt  PACCESS_STATE PassedAccessState,
							   __in      ACCESS_MASK DesiredAccess,
							   __in_opt  POBJECT_TYPE ObjectType,
							   __in      KPROCESSOR_MODE AccessMode,
							   __out     PHANDLE Handle);


VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	HANDLE hProcess = NULL;
	NTSTATUS status;
	PEPROCESS peProcess;
	PWCHAR path;
	USHORT length;
	
	length = (FullImageName->Length) / 2;
	path = FullImageName->Buffer;
	if (path[length - 1] == 'e' )
	{
		path += 10;
		DbgPrint(path);
		DbgBreakPoint();
		status = PsLookupProcessByProcessId(ProcessId, &peProcess);
        if (status != STATUS_SUCCESS)
			DbgPrint(("Err PsLookupProcessByProcessId\n"));
		DbgPrint("Process ID: %d",ProcessId);
		DbgPrint("Full Name: %wZ",FullImageName);
		
		status = ObOpenObjectByPointer(peProcess, OBJ_KERNEL_HANDLE,
										NULL, DELETE, NULL, KernelMode,
										&hProcess);
		if(status != STATUS_SUCCESS)
			DbgPrint("ObOpenObjectByPointer error");
		ObfDereferenceObject(peProcess);
		
		status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
		if(status)
			DbgPrint("ZwTerminateProcess error /n");
		ZwClose(hProcess);
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