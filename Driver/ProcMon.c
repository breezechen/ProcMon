#include <ntifs.h>
#include <wcstr.h>
#include "ProcMon.h"
#include "List.h"
#include "Debug.h"
#include "Hash.h"

#ifdef _WIN64
typedef ULONGLONG ADDR_PTR;
#else
typedef ULONG ADDR_PTR;
#endif

// Global variables
UNICODE_STRING DeviceName;
UNICODE_STRING SymbolicLinkName;
PDEVICE_OBJECT deviceObject;
LIST_ENTRY ListHead; // rules list head
HANDLE gProcessId; // global pid
KEVENT kEvent; // event for sync CreateProc & LoadIm
KMUTEX kMutex; // sync work with list
KSTART_ROUTINE ThreadStart;
HANDLE hThread;

// Driver functions

NTSTATUS DriverCreateClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS DriverIoControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

// Process functions

VOID ProcessCallback( IN HANDLE  hParentId, IN HANDLE  hProcessId, IN BOOLEAN bCreate );

VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

PVOID GetInfoTable(VOID);

VOID GetNativeProcessList(OUT PVOID Buffer, IN ULONG BufferSize, IN ULONG *MemSize );

PWCHAR GetProcessName(IN PUNICODE_STRING FullImageName);


VOID LoadImageNotifyRoutine( PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	PWCHAR pProcName;
	PEPROCESS peProcess;
	ProcessList NewEntry;
	ULONG hash = 0;
	NTSTATUS status;

	if (gProcessId != ProcessId) return;
	gProcessId = 0;
	KeSetEvent(&kEvent,IO_NO_INCREMENT,FALSE);

	pProcName = GetProcessName(FullImageName);
	hash = GetHashCode(FullImageName);
	InitEntry(&NewEntry,pProcName,NULL,&hash);

	KeWaitForMutexObject(&kMutex,Executive,KernelMode,FALSE,NULL);
	if ( !SearchEntry(&ListHead,NewEntry )) 
	{
		KeReleaseMutex(&kMutex,FALSE);
		return;
	}
	KeReleaseMutex(&kMutex,FALSE);
		
	#if DBG
		DbgPrint("Process ID: %d\n",ProcessId);
		DbgPrint("Catched proc: %ws\n",pProcName);
	#endif

	status = PsLookupProcessByProcessId(ProcessId, &peProcess);
	if (!NT_SUCCESS(status))
		{
			#if DBG
				DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "PsLookupProcessByProcessId", status);
				WriteLog("Err PsLookupProcessByProcessId");
			#endif
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
			DbgPrint("Catch process created, PID: %d\n",ProcessId);
			#endif

		}

}

VOID DriverUnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
	// Turn OFF notify callbacks
	PsSetCreateProcessNotifyRoutine(ProcessCallback, TRUE);
	PsRemoveLoadImageNotifyRoutine(  LoadImageNotifyRoutine );

	// Clean rule list
	DeleteAll(&ListHead);

	// Delete link, device
	IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(deviceObject);

	#if DBG
		DbgPrint("Unload!\n");
		WriteLog("Unload ProcMon");
		CloseLogFile();
	#endif
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,  IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	PDRIVER_DISPATCH *ppdd;
	PCWSTR dDeviceName       = L"\\Device\\procmon";
	PCWSTR dSymbolicLinkName = L"\\DosDevices\\procmon";

	InitializeListHead(&ListHead);

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
	    DriverObject->DriverUnload = DriverUnloadRoutine;

	ppdd = DriverObject->MajorFunction;  
	ppdd [IRP_MJ_CREATE] = DriverCreateClose;
    ppdd [IRP_MJ_CLOSE ] = DriverCreateClose;
    ppdd [IRP_MJ_DEVICE_CONTROL ] = DriverIoControl;

	//turn on notify callbacks
    PsSetCreateProcessNotifyRoutine(ProcessCallback, FALSE);
	PsSetLoadImageNotifyRoutine( LoadImageNotifyRoutine );

	// initialize and set ivent for CreateProc and LoadImage syncronyzation
	KeInitializeEvent(&kEvent, SynchronizationEvent, TRUE);
	KeSetEvent(&kEvent,IO_NO_INCREMENT,FALSE);

	KeInitializeMutex(&kMutex,1);

	// Initialize table for CRC32
	InitHashTable();

	#if DBG
		OpenLogFile();
		WriteLog("Driver loaded");
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
	PProcessList pProcRec;
	ProcessList ProcRec;
	//-------------------------------
	ULONG ControlCode =
	IrpStack->Parameters.DeviceIoControl.IoControlCode;

	PWCHAR procName;
	ULONG inSize;


	switch (ControlCode)
	{
	case IOCTL_GET_PROCLIST:
		{
			#if DBG
			DbgPrint("Get Process List request\n");
			WriteLog("Retrieving process List");
			#endif
			GetNativeProcessList(Irp->AssociatedIrp.SystemBuffer, 
								IrpStack->Parameters.DeviceIoControl.OutputBufferLength, 
								&Irp->IoStatus.Information);
			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_ADD_RULE:
		{
			pProcRec = (PProcessList)ExAllocatePool(NonPagedPool, sizeof(ProcessList));
			if (IrpStack->Parameters.DeviceIoControl.InputBufferLength >= sizeof(ProcessList))
			{
				RtlCopyBytes(pProcRec,Irp->AssociatedIrp.SystemBuffer,sizeof(ProcessList));
			}
			else
			{
				#if DBG
					DbgPrint("Small size of input buffer\n");
					WriteLog("Small size of input buffer");
				#endif
				break;
			}

			KeWaitForMutexObject(&kMutex,Executive,KernelMode,FALSE,NULL);
			InsertTailList(&ListHead, &(pProcRec->ListEntry));
			KeReleaseMutex(&kMutex,FALSE);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_DELETE_RULE:
		{
			procName = (PWCHAR)Irp->AssociatedIrp.SystemBuffer;
			inSize = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
			#if DBG
			DbgPrint("Delete rule: %ws\n",procName);
			#endif
			RtlCopyBytes(&(ProcRec.ProcessName),procName,inSize);

			KeWaitForMutexObject(&kMutex,Executive,KernelMode,FALSE,NULL);
			DeleteEntry(&ListHead,ProcRec);
			KeReleaseMutex(&kMutex,FALSE);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_CLEAN_LIST:
		{
			KeWaitForMutexObject(&kMutex,Executive,KernelMode,FALSE,NULL);
			DeleteAll(&ListHead);
			KeReleaseMutex(&kMutex,FALSE);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_DBG_PRINT_LIST:
		{
			KeWaitForMutexObject(&kMutex,Executive,KernelMode,FALSE,NULL);
			PrintAll(&ListHead);
			KeReleaseMutex(&kMutex,FALSE);

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

	status = ObOpenObjectByPointer(peProcess, OBJ_KERNEL_HANDLE,
										NULL, DELETE, NULL, KernelMode,
										&hProcess);
	if(!NT_SUCCESS(status))
			{
				#if DBG
					DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "ObOpenObjectByPointer", status);
					WriteLog("ObOpenObjectByPointer error");
				#endif
				ZwClose(hThread);
				PsTerminateSystemThread(STATUS_SUCCESS);
			}
	status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
	if(!NT_SUCCESS(status))
			{
				#if DBG
				DbgPrint("***(%s line: %d) ***\n\t%s success with code %#x\n\n", __FILE__, __LINE__, "ZwTerminateProcess", status);
				#endif
			}
	ObDereferenceObject(peProcess);
	ZwClose(hProcess);
	ZwClose(hThread);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

PVOID GetInfoTable()
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
			St = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, mPtr, mSize, NULL); 
		} else return NULL;
		if (St == STATUS_INFO_LENGTH_MISMATCH)
		{
			#if DBG
			DbgPrint("Error to get table of process info\n");
			#endif

			ExFreePool(mPtr);
			mSize = mSize * 2;
		}
	} while (St == STATUS_INFO_LENGTH_MISMATCH);
	if (St == STATUS_SUCCESS) return mPtr;
	ExFreePool(mPtr);
	return NULL;
}

VOID GetNativeProcessList(OUT PVOID Buffer, IN ULONG BufferSize, IN ULONG *MemSize )
{
	PVOID Info;
	PSYSTEM_PROCESSES Proc;
	PProcessRecord Data;
	PVOID Mem = NULL;
	ULONG PsCount = 0;


	Info = GetInfoTable();
	if(!Info)
	{
		#if DBG
		WriteLog("Unable get a ProcessInfo Table\n");
		#endif 

		return;
	}
	else Proc = (PSYSTEM_PROCESSES)Info;
	do 
	{
		Proc = (PSYSTEM_PROCESSES)((ADDR_PTR)Proc + Proc->NextEntryDelta);	
		PsCount++;
	} while (Proc->NextEntryDelta);

	*MemSize = (PsCount + 1) * sizeof(TProcessRecord);

	if (BufferSize < *MemSize)
	{
		#if DBG
			DbgPrint("Small buffer size\n");
			WriteLog("Small buffer size\n");
		#endif

		return;
	}
	Mem = ExAllocatePool(PagedPool, *MemSize);
	if(!Mem)
		return;
	else
		Data = (PProcessRecord)Mem;
	Proc = (PSYSTEM_PROCESSES)Info;
	do
	{
		Proc = (PSYSTEM_PROCESSES)((ADDR_PTR)Proc + Proc->NextEntryDelta);
		wcscpy(Data->ProcessName,Proc->ProcessName.Buffer);

		#if DBG 
		DbgPrint("%ws\n",Data->ProcessName);
		#endif

		Data->ProcessId  = Proc->ProcessId;
		Data++;
	} while (Proc->NextEntryDelta);
	ExFreePool(Info);
	RtlCopyBytes(Buffer,Mem,*MemSize);
	ExFreePool(Mem);
	return;
}

PWCHAR GetProcessName(IN PUNICODE_STRING FullImageName)
{
	USHORT i;	

	i = ((FullImageName->Length)/2) - 1;
	
	while (i>0)
		{
			if ( *(FullImageName->Buffer+i) == '\\')
				break;
			i--;
		}	

	return FullImageName->Buffer+(++i);
}