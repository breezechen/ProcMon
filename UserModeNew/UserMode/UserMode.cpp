// UserMode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Driver.h"
#include <string>
#include <iostream>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windows.h>

using namespace std;


void setup()
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
		cout<<"SC MAnager isn't start";
		return; 
	}
	DWORD k = GetLastError(); 

	SC_HANDLE t = CreateService(man, L"procmon1", L"procmon1", SERVICE_START | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, namebuff, 0, 0, 0, 0, 0);

	if  (t == NULL)
	{
		cout<<"Service isn't create";
	}
	k = GetLastError();
	StartService(t, 0, 0);
	k = GetLastError();
	CloseServiceHandle(t);
}

void cleanup()
{	
	DWORD k = 0;
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	SERVICE_STATUS stat;
	SC_HANDLE t = OpenService(man, L"procmon1", SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	if (t == NULL)
	{
		return; 
	}
	ControlService(t, SERVICE_CONTROL_STOP, &stat);   
	k = GetLastError();
	DeleteService(t);
	k = GetLastError();
	CloseServiceHandle(t);
}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hDevice;
	BOOL status;
	DWORD	BufSize = 0x2000;
	TProcessRecord	*buf = 0;
	int select = 0;
	DWORD BytesReturned = 0;    
	buf = new TProcessRecord [50];
	unsigned long ioctlCode=IOCTL_GET_PROCESS_LIST;


	while (true)
	{
		printf("\nSelect action \n1 - Load Driver and open \n2 - Get Proc List \n3 - Add Rule \n4 - Close Driver and Exit \nYour choise: ");
		cin>>select;
		switch (select)
		{
			case 1:
				{
					setup();
					hDevice = CreateFile(L"\\\\.\\ProcMon",
												GENERIC_READ | GENERIC_WRITE,
												0,		// share mode none
												NULL,	// no security
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL,
												NULL );		// no template
					if (hDevice == INVALID_HANDLE_VALUE) {
							printf("Failed to obtain file handle to device: "
									"%s with Win32 error code: %d\n",
									"LBK1", GetLastError() );
					}

					break;
				}

			case 2:
			{
				if( !DeviceIoControl(   hDevice,
					ioctlCode,
					NULL, 0,	// Input
					buf, BufSize,	// Output
					&BytesReturned,
					NULL )  )
				{
					printf( "Error in IOCTL_PRINT_DEBUG_MESS!" );
					return(-1);
				}
				int i = 0;
				int prCount = (BytesReturned/sizeof(TProcessRecord) - 1);
				while (i<prCount)
				{
					printf("PID: %d\t Name: %s\n",(buf+i)->ProcessId,(buf+i)->ProcessName);
					i++;
				}
				delete[] buf;
				break;
			}
			case 3:
				{
					if( !DeviceIoControl(   hDevice,
						IOCTL_ADD_RULE,
						NULL, 0,	// Input
						NULL, 0,	// Output
						&BytesReturned,
						NULL )  )
					{
						printf( "Error in IOCTL_PRINT_DEBUG_MESS!" );
						return(-1);
					}
					break;
				}
			case 4:
				{
					status = CloseHandle(hDevice);
					if (!status) {
						printf("Failed on call to CloseHandle - error: %d\n",
						GetLastError() );
						return 6;
					}
					printf("Succeeded in closing device...exiting normally\n");

					cleanup();
					return 0;
				}
		}

	
	}

}