
#include <NTDDK.h>


#define IOCTL_GET_PROCESS_LIST CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ADD_RULE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _ProcessRecord
{
	ULONG       ProcessId;
	CHAR        ProcessName[256]; 
} TProcessRecord, *PProcessRecord;

typedef struct {
  PWCHAR fullName;
  PWCHAR name;
  PWCHAR hash;
  SINGLE_LIST_ENTRY SingleListEntry;
  ULONG pId;
} BLIST_ENTRY, *PBLIST_ENTRY;

typedef enum _SYSTEM_INFORMATION_CLASS 
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation, 
	SystemNotImplemented1,
	SystemProcessesAndThreadsInformation,
	SystemCallCounts, 
	SystemConfigurationInformation, 
	SystemProcessorTimes, 
	SystemGlobalFlag, 
	SystemNotImplemented2, 
	SystemModuleInformation, 
	SystemLockInformation,
	SystemNotImplemented3, 
	SystemNotImplemented4, 
	SystemNotImplemented5, 
	SystemHandleInformation, 
	SystemObjectInformation, 
	SystemPagefileInformation, 
	SystemInstructionEmulationCounts, 
	SystemInvalidInfoClass1, 
	SystemCacheInformation, 
	SystemPoolTagInformation, 
	SystemProcessorStatistics,
	SystemDpcInformation, 
	SystemNotImplemented6,
	SystemLoadImage, 
	SystemUnloadImage, 
	SystemTimeAdjustment, 
	SystemNotImplemented7, 
	SystemNotImplemented8, 
	SystemNotImplemented9,
	SystemCrashDumpInformation, 
	SystemExceptionInformation, 
	SystemCrashDumpStateInformation, 
	SystemKernelDebuggerInformation, 
	SystemContextSwitchInformation, 
	SystemRegistryQuotaInformation, 
	SystemLoadAndCallImage,
	SystemPrioritySeparation, 
	SystemNotImplemented10,
	SystemNotImplemented11, 
	SystemInvalidInfoClass2, 
	SystemInvalidInfoClass3, 
	SystemTimeZoneInformation, 
	SystemLookasideInformation, 
	SystemSetTimeSlipEvent,
	SystemCreateSession,
	SystemDeleteSession, 
	SystemInvalidInfoClass4, 
	SystemRangeStartInformation, 
	SystemVerifierInformation, 
	SystemAddVerifier, 
	SystemSessionProcessesInformation 
} SYSTEM_INFORMATION_CLASS;
typedef struct _SYSTEM_THREADS 
{
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
	ULONG ContextSwitchCount;
	ULONG State;
	KWAIT_REASON WaitReason;
} SYSTEM_THREADS, *PSYSTEM_THREADS;
typedef struct _SYSTEM_PROCESSES 
{
	ULONG NextEntryDelta;
	ULONG ThreadCount;
	ULONG Reserved1[6];
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ProcessName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromProcessId;
	ULONG HandleCount;
	ULONG Reserved2[2];
	VM_COUNTERS VmCounters;
	IO_COUNTERS IoCounters; 
	SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;