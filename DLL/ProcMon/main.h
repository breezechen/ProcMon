#ifndef MYDLLH
#define MYDLLH

// Returns a + b
extern "C" _declspec(dllexport) void setup();
extern "C" _declspec(dllexport) void StartDriver();
extern "C" _declspec(dllexport) void StopDriver();
extern "C" _declspec(dllexport) void DeleteDriver();
extern "C" _declspec(dllexport) int OpenDevice(HANDLE &hDevice);
extern "C" _declspec(dllexport) int GPL(const HANDLE hDevice, std::list<TProcessRecord> &list);
extern "C" _declspec(dllexport) int newRule(const HANDLE hDevice, ProcessList proc);
extern "C" _declspec(dllexport) int deleteRule(const HANDLE hDevice, ProcessList proc);
extern "C" _declspec(dllexport) int PrintDriverRules(const HANDLE hDevice);
extern "C" _declspec(dllexport) int deleteDriverRules(const HANDLE hDevice);
extern "C" _declspec(dllexport) int CloseDriver_(HANDLE &hDevice);
#endif