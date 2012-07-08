#include <ntifs.h>

UNICODE_STRING DeviceName;
UNICODE_STRING SymbolicLinkName;
PDEVICE_OBJECT deviceObject = NULL;
#define BLACK_LIST_LOAD CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

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
	PWCHAR name;
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
			ObDereferenceObject(peProcess);
		
			status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
			if(status)
				DbgPrint("ZwTerminateProcess error /n");
			ZwClose(hProcess);
		}
	}
	}

}

NTSTATUS DriverCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID UnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
    PsRemoveLoadImageNotifyRoutine(  LoadImageNotifyRoutine );
	IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(deviceObject);
    DbgPrint("Unload!\n");
}

NTSTATUS DriverIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION pisl;
    NTSTATUS ns = STATUS_UNSUCCESSFUL;
    ULONG BuffSize, DataSize = 3;
	PWCHAR pBuff = NULL, pData = L"out\0";
   
    pisl = IoGetCurrentIrpStackLocation (Irp);
	BuffSize = pisl->Parameters.DeviceIoControl.OutputBufferLength;
	pBuff = Irp->AssociatedIrp.SystemBuffer;
	Irp->IoStatus.Information = 0;
	
	DbgPrint(pBuff);
	DbgPrint(pData);
	
	DbgBreakPoint();

	switch(pisl->Parameters.DeviceIoControl.IoControlCode)
	{
		case BLACK_LIST_LOAD :
		   //pData = GetEprocessProcessList(&DataSize);
		   if (pData)
		   {
			   if (BuffSize >= DataSize)
			   {
				   memcpy(pBuff, pData, DataSize);
				   Irp->IoStatus.Information = DataSize;
				   DbgPrint(pBuff);
				   ns = STATUS_SUCCESS;
			   } else ns = STATUS_INFO_LENGTH_MISMATCH;
			  //ExFreePool(pData);
		   }
		 break;
		 }   

    Irp->IoStatus.Status = ns;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return ns;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,  IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	PDRIVER_DISPATCH *ppdd;
	PCWSTR dDeviceName       = L"\\Device\\procmon";
	PCWSTR dSymbolicLinkName = L"\\DosDevices\\procmon";

	RtlInitUnicodeString(&DeviceName,       dDeviceName);
    RtlInitUnicodeString(&SymbolicLinkName, dSymbolicLinkName);
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &deviceObject);
	if (!NT_SUCCESS(status)) return status;

	status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(deviceObject);
		return status;
	}
	
    PsSetLoadImageNotifyRoutine( LoadImageNotifyRoutine );
    DriverObject->DriverUnload = UnloadRoutine;
	ppdd = DriverObject->MajorFunction;
   
	ppdd [IRP_MJ_CREATE] = DriverCreateClose;
    ppdd [IRP_MJ_CLOSE ] = DriverCreateClose;
    ppdd [IRP_MJ_DEVICE_CONTROL ] = DriverIoControl;
    DbgPrint("Driver loaded");
    
    return status; 

}