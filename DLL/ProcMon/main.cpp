// UserMode.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <list>
#include <stdio.h>
#include "Driver.h"
#include "main.h"
#include "log.h"
#include <string>
#include <iostream>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

using namespace std;


extern "C" _declspec(dllexport) void setup()
{
	WCHAR namebuff[MAX_PATH];
	//get path to ths .sys.file
	GetModuleFileNameW(0, namebuff,256);
	DWORD a = wcslen( (const wchar_t*)namebuff ) ;
	while(1)
	{
	  if(namebuff[a] == '\\')
		  break;
	  a--;
	}
	a++;
	wcscpy(&namebuff[a], L"ProcMon.sys");
	//create service
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (man == NULL)
	{
		log("SC MAnager isn't start\n");
		return; 
	}
	SC_HANDLE t = CreateService(man, L"procmon", L"procmon", SERVICE_START | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, namebuff, 0, 0, 0, 0, 0);

	if  (t == NULL)
	{
		log("Service isn't create\n");
	}
	CloseServiceHandle(t);
}

extern "C" _declspec(dllexport) void StartDriver()
{
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (man == NULL)
	{
		log("Cant open SCManager(StartDriver)\n");
		return; 
	}
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	if (t == NULL)
	{
		log("Cant open Service(StartDriver)\n");
		return; 
	}
	if(StartService(t, 0, 0))
		log("Cant start Service(StartDriver)\n");
	CloseServiceHandle(t);
}
extern "C" _declspec(dllexport) void StopDriver()
{	
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if(man)
	{
		log("Cant open SCManager(StopDriver)\n");
		return;
	}
	SERVICE_STATUS stat;
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	if (t == NULL)
	{
		log("Cant open Service(StopDriver)\n");
		return; 
	}
	if(ControlService(t, SERVICE_CONTROL_STOP, &stat))
		log("Cant stop service(StopDriver)\n");   
	CloseServiceHandle(t);
}
extern "C" _declspec(dllexport) void DeleteDriver()
{
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if(man)
	{
		log("Cant open SCManager(DeleteDriver)\n");
		return;
	}
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	if (t == NULL)
	{
		log("Cant open Service(DeleteDriver)\n");
		return; 
	}
		DeleteService(t);
}

extern "C" _declspec(dllexport) int SOD()
{
	StartDriver();
	HANDLE hDevice = NULL;
	hDevice = CreateFile(L"\\\\.\\procmon",
												GENERIC_READ | GENERIC_WRITE,
												0,		// share mode none
												NULL,	// no security
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL,
												NULL );		// no template
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		log("Cant get device handle(SOD)\n");
		hDevice = NULL;
		return 0;
	}
	log("Driver start SUCCESS(SOD)\n");
	return (int)hDevice;
}

extern "C" _declspec(dllexport) int GPL(int hDevice, std::list<TProcessRecord*> &list)
{
	DWORD BytesReturned = 0;
	DWORD	BufSize = 100 * sizeof(TProcessRecord);
	BytesReturned = 0;
	TProcessRecord* buf = NULL;
	buf = new TProcessRecord[100];
	if( !DeviceIoControl(   (HANDLE)hDevice,
							IOCTL_GET_PROCLIST,
							NULL, 0,	// Input
							buf, BufSize,	// Output
							&BytesReturned,
							NULL )  )
	{
		log( "Error at geting process list(GPL)\n" );
		delete[] buf;
		return 0;
	}
	int countStruct = BytesReturned / sizeof(TProcessRecord);
	while(countStruct--)
		list.push_front(&buf[countStruct]);
	delete[] buf;
	return 1; 
}

extern "C" _declspec(dllexport) int newRule(int hDevice, wchar_t *procName)
{
	DWORD BytesReturned = 0;
	if( !DeviceIoControl(   (HANDLE)hDevice,
							IOCTL_ADD_RULE,
							&procName, sizeof(procName),	// Input
							NULL, 0,	// Output
							&BytesReturned,
							NULL )  )
	{
		log( "Error at sending new rule to driver(newRule)\n" );
		return 0;
	}
	return 1;
}

extern "C" _declspec(dllexport) int deleteRule(int hDevice, wchar_t *procName)
{
	DWORD BytesReturned = 0;
	if( !DeviceIoControl((HANDLE)hDevice,
						IOCTL_DELETE_RULE,
						&procName, sizeof(procName),	// Input
						NULL, 0,	// Output
						&BytesReturned,
						NULL ))
	{
		printf( "Error at  sending deleteRule(deleteRule)\n" );
		return 0;
	}
	return 1;
}

int PrintDriverRules(int hDevice)
{
	DWORD BytesReturned = 0;
	if( !DeviceIoControl(   (HANDLE)hDevice,
							IOCTL_DBG_PRINT_LIST,
							NULL, 0,	// Input
							NULL, 0,	// Output
							&BytesReturned,
							NULL )  )
	{
		printf( "Error at sending PrintDriverRules(PrintDriverRules)\n" );
		return 0;
	}
	return 1;
}

extern "C" _declspec(dllexport) int deleteDriverRules(int hDevice)
{
	DWORD BytesReturned = 0;
	if( !DeviceIoControl(   (HANDLE)hDevice,
							IOCTL_CLEAN_LIST,
							NULL, 0,	// Input
							NULL, 0,	// Output
							&BytesReturned,
							NULL )  )
	{
	printf( "Error at deleting driver rule(deleteDriverRules)\n" );
	return 0;
	}
	return 1;
}

extern "C" _declspec(dllexport) int CSD(int hDevice)
{
	BOOL status;
	if ((HANDLE)hDevice != NULL)
	{
		status = CloseHandle((HANDLE)hDevice);
		if (!status) 
		{
			log("Failed on call to CloseHandle(CSD)\n");
			return 0;
		}
		printf("Succeeded in closing device...exiting normally\n");
	}
	StopDriver();
	log("Driver stoped SUCCESS(CSD)\n");
	return 1;
}


