#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windows.h>


HANDLE device;

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
	wcscpy(&namebuff[a], L"LoadImage.sys");
	//create service
	SC_HANDLE man = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (man == NULL) 
		return; 
	DWORD k = GetLastError(); 

	SC_HANDLE t = CreateService(man, L"procmon", L"procmon", SERVICE_START | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, namebuff, 0, 0, 0, 0, 0);

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
	SC_HANDLE t = OpenService(man, L"procmon", SC_MANAGER_ALL_ACCESS);
	k = GetLastError();
	if (t == NULL) 
		return; 
	ControlService(t, SERVICE_CONTROL_STOP, &stat);   
	k = GetLastError();
	DeleteService(t);
	k = GetLastError();
	CloseServiceHandle(t);
}

void go()
{
	setup();
	PWCHAR controlbuff = L"im here\0";
	DWORD dw = GetLastError();

	std::cout << *controlbuff << "\n";
	//open device
	device = CreateFile(L"\\\\.\\procmon",GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	dw = GetLastError();
	DeviceIoControl(device, 1000, &controlbuff, 256, &controlbuff, 256, &dw, 0);
	dw = GetLastError();
	std::cout << *controlbuff;
}

int main()
{
	go();
	int i;
	std::cin >> i;
	cleanup();
	return 0;
}