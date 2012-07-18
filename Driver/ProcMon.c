#include <ntifs.h>
#include "ProcMon.h"
#include <wcstr.h>


// Global variables
UNICODE_STRING notepad;
UNICODE_STRING DeviceName;
UNICODE_STRING SymbolicLinkName;
PDEVICE_OBJECT deviceObject = NULL;
HANDLE gProcessId;
KEVENT kEvent;
KSTART_ROUTINE ThreadStart;
HANDLE hThread;


// Functions

VOID ProcessCallback( IN HANDLE  hParentId, IN HANDLE  hProcessId, IN BOOLEAN bCreate );

static VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

NTSTATUS DriverCreateClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS DriverIoControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

PVOID GetInfoTable(ULONG ATableType);

VOID GetNativeProcessList(PVOID Buffer, ULONG *MemSize );

NTSTATUS ZwQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,IN OUT PVOID SystemInformation,IN ULONG SystemInformationLength,OUT PULONG ReturnLength OPTIONAL);


VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	USHORT i;
	UNICODE_STRING ProcName;
	PEPROCESS peProcess;

	if (gProcessId != ProcessId) return;
	gProcessId = 0;
	KeSetEvent(&kEvent,IO_NO_INCREMENT,FALSE);

	i = ((FullImageName->Length)/2) - 1;
	
	while (i>0)
		{
			if ( *(FullImageName->Buffer+i) == '\\')
				break;
			i--;
		}
	RtlInitUnicodeString(&ProcName,(FullImageName->Buffer+(++i)));

	if ( RtlCompareUnicodeString(&ProcName,&notepad,TRUE) != 0 ) return;
		
	#if DBG
	DbgPrint("Process ID: %d\n",ProcessId);
	DbgPrint("Catched proc: %wZ\n",&ProcName);
	#endif

	if (PsLookupProcessByProcessId(ProcessId, &peProcess) != STATUS_SUCCESS)
			{
				DbgPrint("Err PsLookupProcessByProcessId\n");
				return;
			}
	PsCreateSystemThread(&hThread,THREAD_ALL_ACCESS,NULL,NULL,NULL,&ThreadStart,peProcess);	
}

VOID ProcessCallback(IN HANDLE  ParentId, IN HANDLE  ProcessId, IN BOOLEAN bCreate )
{
		if (bCreate)
		{
			KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);
			gProcessId = ProcessId;

			#if DBG
			DbgPrint("create %d\n",ProcessId);
			#endif

		}

}

VOID DriverUnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
    PsSetCreateProcessNotifyRoutine(ProcessCallback, TRUE);
	PsRemoveLoadImageNotifyRoutine(  LoadImageNotifyRoutine );

	IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(deviceObject);

	#if DBG
    DbgPrint("Unload!\n");
	#endif
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

	//turn on notify callbacks
    PsSetCreateProcessNotifyRoutine(ProcessCallback, FALSE);
	PsSetLoadImageNotifyRoutine( LoadImageNotifyRoutine );

	// this is test stab
	RtlInitUnicodeString(&notepad, L"notepad.exe");

    DriverObject->DriverUnload = DriverUnloadRoutine;

	ppdd = DriverObject->MajorFunction;  
	ppdd [IRP_MJ_CREATE] = DriverCreateClose;
    ppdd [IRP_MJ_CLOSE ] = DriverCreateClose;
    ppdd [IRP_MJ_DEVICE_CONTROL ] = DriverIoControl;

	// initialize and set ivent for CreateProc and LoadImage syncronyzation
	KeInitializeEvent(&kEvent, SynchronizationEvent, TRUE);
	KeSetEvent(&kEvent,IO_NO_INCREMENT,FALSE);

	#if DBG
    DbgPrint("Driver loaded\n");
    #endif

    return status; 

}

NTSTATUS DriverCreateClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DriverIoControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpStack=IoGetCurrentIrpStackLocation(Irp);

	//-------------------------------
	ULONG ControlCode =
	IrpStack->Parameters.DeviceIoControl.IoControlCode;

	// 

	switch (ControlCode)
	{
	case IOCTL_GET_PROCESS_LIST:
		{
			#if DBG
			DbgPrint("Get Process List request\n");
			#endif
			GetNativeProcessList(Irp->AssociatedIrp.SystemBuffer, &Irp->IoStatus.Information);
			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_ADD_RULE:
		{
			DbgPrint("Add rule");
			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	default: 
		{
			Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			Irp->IoStatus.Information = 0;
		}
	}


    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID ThreadStart( __in PEPROCESS peProcess)

{
	NTSTATUS status;
	HANDLE hProcess = NULL;

	ObOpenObjectByPointer(peProcess, OBJ_KERNEL_HANDLE,
										NULL, DELETE, NULL, KernelMode,
										&hProcess);
	status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
	if(status)
			{
				DbgPrint("ZwTerminateProcess error /n");
				return;
			}
	ObDereferenceObject(peProcess);
	ZwClose(hProcess);
	ZwClose(hThread);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

PVOID GetInfoTable(ULONG ATableType)
{
	ULONG mSize = 0x4000;
	PVOID mPtr = NULL;
	NTSTATUS St;
	do
	{
		mPtr = ExAllocatePool(PagedPool, mSize);
		memset(mPtr, 0, mSize);
		if (mPtr) 
		{
			St = ZwQuerySystemInformation(ATableType, mPtr, mSize, NULL); 
		} else return NULL;
		if (St == STATUS_INFO_LENGTH_MISMATCH)
		{
			ExFreePool(mPtr);
			mSize = mSize * 2;
		}
	} while (St == STATUS_INFO_LENGTH_MISMATCH);
	if (St == STATUS_SUCCESS) return mPtr;
	ExFreePool(mPtr);
	return NULL;
}

VOID GetNativeProcessList(PVOID Buffer, ULONG *MemSize)
{
	PVOID Info;
	PSYSTEM_PROCESSES Proc;
	PProcessRecord Data;
	PVOID Mem = NULL;
	ULONG PsCount = 0;


	Info = GetInfoTable(SystemProcessesAndThreadsInformation);
	if(!Info)
		return;
	else Proc = Info;
	do 
	{
		Proc = (PSYSTEM_PROCESSES)((ULONG)Proc + Proc->NextEntryDelta);	
		PsCount++;
	} while (Proc->NextEntryDelta);

	*MemSize = (PsCount + 1) * sizeof(TProcessRecord);
	Mem = ExAllocatePool(PagedPool, *MemSize);
	if(!Mem)
		return;
	else
		Data = Mem;
	Proc = Info;
	do
	{
		Proc = (PSYSTEM_PROCESSES)((ULONG)Proc + Proc->NextEntryDelta);
		wcstombs(Data->ProcessName, Proc->ProcessName.Buffer, 255);

		#if DBG 
		DbgPrint("%s\n",Data->ProcessName);
		#endif

		Data->ProcessId  = Proc->ProcessId;
		Data++;
	} while (Proc->NextEntryDelta);
	ExFreePool(Info);
	RtlCopyBytes(Buffer,Mem,*MemSize);
	ExFreePool(Mem);
	return;
}

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