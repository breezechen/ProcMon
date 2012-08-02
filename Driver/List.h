
#include <NTDDK.h>


typedef struct _ProcessList
{
	ULONG		Hash;
	USHORT      ProcessId;
	WCHAR       ProcessName[256];
	LIST_ENTRY ListEntry;
} ProcessList, *PProcessList;

BOOLEAN SearchEntry( IN PLIST_ENTRY ListHead, IN ProcessList SearchProc);

VOID PrintAll( IN PLIST_ENTRY ListHead );

VOID DeleteEntry( IN PLIST_ENTRY ListHead, IN ProcessList DeleteProc);

VOID DeleteAll( IN PLIST_ENTRY ListHead );

VOID InitEntry( OUT PProcessList pNewEntry, IN PWCHAR pProcessName, PUSHORT pProcessId, PULONG pHash);
