#ifndef MYDLLH
#define MYDLLH

// Returns a + b
extern "C" _declspec(dllexport) void setup();
extern "C" _declspec(dllexport) void StartDriver();
extern "C" _declspec(dllexport) void StopDriver();
extern "C" _declspec(dllexport) void DeleteDriver();
extern "C" _declspec(dllexport) int SOD();
extern "C" _declspec(dllexport) int GPL(int hDevice, std::list<TProcessRecord*> &list);
extern "C" _declspec(dllexport) int newRule(int hDevice, wchar_t *procName);
extern "C" _declspec(dllexport) int deleteRule(int hDevice, wchar_t *procName);
extern "C" _declspec(dllexport) int PrintDriverRules(int hDevice);
extern "C" _declspec(dllexport) int deleteDriverRules(int hDevice);
extern "C" _declspec(dllexport) int CSD(int hDevice);
#endif