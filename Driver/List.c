
#include "ProcMon.h"
#include "List.h"


BOOLEAN SearchEntry(IN PLIST_ENTRY ListHead, IN ProcessList SearchProc);

VOID PrintAll(IN PLIST_ENTRY ListHead);

VOID DeleteEntry(IN PLIST_ENTRY ListHead, IN ProcessList DeleteProc);

VOID DeleteAll(IN PLIST_ENTRY ListHead);

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
        
		if (!wcscmp(pProcRec->ProcessName,SearchProc.ProcessName))
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
		DbgPrint("%ws\n",pProcRec->ProcessName);
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