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

	SC_HANDLE t = CreateService(man, L"procmon", L"procmon", SERVICE_START | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, namebuff, 0, 0, 0, 0, 0);

	if  (t == NULL)
	{
		cout<<"Service isn't create";
	}
	k = GetLastError();
	CloseServiceHandle(t);
}

void StartDriver()
{
	DWORD k = 0;
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	if (t == NULL)
	{
		return; 
	}
	StartService(t, 0, 0);
	k = GetLastError();
	CloseServiceHandle(t);
}
void StopDriver()
{	
	DWORD k = 0;
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	SERVICE_STATUS stat;
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	if (t == NULL)
	{
		return; 
	}
	ControlService(t, SERVICE_CONTROL_STOP, &stat);   
	k = GetLastError();
	CloseServiceHandle(t);
}
void DeleteDriver()
{
	DWORD k = 0;
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	if (t == NULL)
	{
		return; 
	}
		DeleteService(t);
}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hDevice = NULL;
	BOOL status;
	DWORD	BufSize = 0x2000;
	TProcessRecord	*buf = 0;
	int select = 0;
	DWORD BytesReturned = 0;    


	while (true)
	{
		printf("\nSelect action \n");
		printf("1 - Setup Driver \n");
		printf("2 - Start Driver and Open Device \n");
		printf("3 - Get Proc List \n");
		printf("4 - Add Rule \n");
		printf("5 - Close Device and Stop Driver \n");
		printf("6 - Delete Driver \n");
		printf("Your choise: ");
		cin>>select;
		switch (select)
		{
			case 1:
				{
					setup();
					break;
				}
			case 2:
				{
					StartDriver();
					hDevice = CreateFile(L"\\\\.\\procmon",
												GENERIC_READ | GENERIC_WRITE,
												0,		// share mode none
												NULL,	// no security
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL,
												NULL );		// no template
					if (hDevice == INVALID_HANDLE_VALUE) {
							printf("Failed to obtain file handle to device: "
									"%s with Win32 error code: %d\n", GetLastError() );
					}
					break;
				}

			case 3:
			{
				BytesReturned = 0; 
				buf = new TProcessRecord [50];
				if( !DeviceIoControl(   hDevice,
					IOCTL_GET_PROCESS_LIST,
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
			case 4:
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
			case 5:
				{
					if (hDevice != NULL)
					{
						status = CloseHandle(hDevice);
						if (!status) {
							printf("Failed on call to CloseHandle - error: %d\n",
							GetLastError() );
							return 6;
						}
						printf("Succeeded in closing device...exiting normally\n");
					}
					StopDriver();
					break;
				}

			case 6:
				{
					DeleteDriver();
					return 0;
				}
		}

	
	}

}


