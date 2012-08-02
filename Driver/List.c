
#include "ProcMon.h"
#include "List.h"


BOOLEAN SearchEntry(IN PLIST_ENTRY ListHead, IN ProcessList SearchProc);

VOID PrintAll(IN PLIST_ENTRY ListHead);

VOID DeleteEntry(IN PLIST_ENTRY ListHead, IN ProcessList DeleteProc);

VOID DeleteAll(IN PLIST_ENTRY ListHead);

VOID InitEntry( OUT PProcessList pNewEntry, IN PWCHAR pProcessName, PUSHORT pProcessId, PULONG pHash);

BOOLEAN SearchEntry(IN PLIST_ENTRY ListHead, IN ProcessList SearchProc)
{     
	PLIST_ENTRY pEntry;     
	PProcessList pProcRec;  

	if (IsListEmpty(ListHead))
	{
		return FALSE;    
	}	
	pEntry = ListHead->Flink;      

	while(pEntry != ListHead)     
	{
		pProcRec = (PProcessList) CONTAINING_RECORD(pEntry, ProcessList, ListEntry);          
		if ((!wcscmp(pProcRec->ProcessName,SearchProc.ProcessName))||
			((pProcRec->Hash == SearchProc.Hash)&&(SearchProc.Hash != 0))||
			((pProcRec->ProcessId == SearchProc.ProcessId)&&(SearchProc.ProcessId != 0)))
		{       
			return TRUE;         
		} 
       
		pEntry = pEntry->Flink;     
	} 
	return FALSE; 
}

VOID PrintAll(IN PLIST_ENTRY ListHead)
{
	PLIST_ENTRY pEntry; 
	PProcessList pProcRec; 

	if (IsListEmpty(ListHead))
		return;  
	pEntry = ListHead->Flink; 
	while(pEntry != ListHead)     
	{

		pProcRec = (PProcessList) CONTAINING_RECORD(pEntry, ProcessList, ListEntry);          
		DbgPrint("Name: %ws\n",pProcRec->ProcessName);
		DbgPrint("PID: %d\n",pProcRec->ProcessId);
		DbgPrint("HASH: %p\n",pProcRec->Hash);
		pEntry = pEntry->Flink;     
	} 

}

VOID DeleteEntry(IN PLIST_ENTRY ListHead, ProcessList DeleteProc)
{
	PLIST_ENTRY pEntry;     
	PProcessList pProcRec;  

	if (IsListEmpty(ListHead))
	{
		return;    
	}
	
	pEntry = ListHead->Flink;      


	while(pEntry != ListHead)     
	{

		pProcRec = (PProcessList) CONTAINING_RECORD(pEntry, ProcessList, ListEntry);          
        
		if (!wcscmp(pProcRec->ProcessName,DeleteProc.ProcessName))      
		{ 
			RemoveEntryList(pEntry);
			return;         
		} 
       
		pEntry = pEntry->Flink;     
	} 
	return; 
}

VOID DeleteAll(IN PLIST_ENTRY ListHead)
{
	while (!IsListEmpty(ListHead))
	{
		RemoveTailList(ListHead);
	}
	
}

VOID InitEntry( OUT PProcessList pNewEntry, IN PWCHAR pProcessName, PUSHORT pProcessId, PULONG pHash)
{
	if  (pProcessName != NULL) 
	{
		wcscpy(pNewEntry->ProcessName,pProcessName);
	}
	else
	{
	}

	if (pProcessId != NULL)
	{
		pNewEntry->ProcessId = *pProcessId;
	}
	else
	{
		pNewEntry->ProcessId = 0;
	}

	if (pHash != NULL)
	{
		pNewEntry->Hash = *pHash;
	}
	else
	{
		pNewEntry->Hash = 0;
	}
}