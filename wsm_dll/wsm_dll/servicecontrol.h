#pragma once
#include <vector>
#include <windows.h>

extern "C" __declspec(dllexport) const wchar_t** EnumerateServices();
extern "C" __declspec(dllexport) int StartServiceC(const char* serviceName);
extern "C" __declspec(dllexport) int StopService(const char* serviceName);
extern "C" __declspec(dllexport) int RestartService(const char* serviceName);
extern "C" __declspec(dllexport) int PauseService(const char* serviceName);
extern "C" __declspec(dllexport) int ContinueService(const char* serviceName);
extern "C" __declspec(dllexport) int ChangeStartType(const char* serviceName, DWORD startType);
extern "C" __declspec(dllexport) const wchar_t* FindServiceByName(const char* serviceName);
extern "C" __declspec(dllexport) const char* GetServiceInfo(const char* serviceName);
extern "C" __declspec(dllexport) DWORD GetServicesCount();