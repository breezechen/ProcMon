#include <windows.h>

extern "C" _declspec(dllexport) ULONG getHash(DWORD);

WINBASEAPI BOOL WINAPI QueryFullProcessImageName(
  __in     HANDLE hProcess,
  __in     DWORD dwFlags,
  __out    LPTSTR lpExeName,
  __inout  PDWORD lpdwSize
);
WINBASEAPI __out HANDLE WINAPI GetCurrentProcess(void);
WINBASEAPI HANDLE WINAPI OpenProcess(
  __in  DWORD dwDesiredAccess,
  __in  BOOL bInheritHandle,
  __in  DWORD dwProcessId
);
WINBASEAPI HANDLE WINAPI CreateFile(
  __in      LPCTSTR lpFileName,
  __in      DWORD dwDesiredAccess,
  __in      DWORD dwShareMode,
  __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  __in      DWORD dwCreationDisposition,
  __in      DWORD dwFlagsAndAttributes,
  __in_opt  HANDLE hTemplateFile
);
WINBASEAPI BOOL WINAPI ReadFile(
  __in         HANDLE hFile,
  __out        LPVOID lpBuffer,
  __in         DWORD nNumberOfBytesToRead,
  __out_opt    LPDWORD lpNumberOfBytesRead,
  __inout_opt  LPOVERLAPPED lpOverlapped
);
WINBASEAPI PIMAGE_NT_HEADERS WINAPI ImageNtHeader(
  __in  PVOID ImageBase
);
WINBASEAPI DWORD WINAPI GetModuleFileNameEx(
  __in      HANDLE hProcess,
  __in_opt  HMODULE hModule,
  __out     LPTSTR lpFilename,
  __in      DWORD nSize
);
ULONG chksum_crc32 (UCHAR *, USHORT);
void InitHashTable();