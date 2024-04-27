#pragma once
#include <vector>
#include <windows.h>

extern "C" __declspec(dllexport) int StartServiceC(const char* serviceName);
extern "C" __declspec(dllexport) int StopService(const char* serviceName);
extern "C" __declspec(dllexport) int PauseService(const char* serviceName);
extern "C" __declspec(dllexport) int ContinueService(const char* serviceName);
extern "C" __declspec(dllexport) int ChangeStartType(const char* serviceName, DWORD startType);
extern "C" __declspec(dllexport) const char* GetServiceInfo(const char* serviceName);
extern "C" __declspec(dllexport) DWORD GetServicesCount();
extern "C" __declspec(dllexport) const char** EnumerateServicesWithInfo();
extern "C" __declspec(dllexport) BOOL CanServiceBePaused(const char* serviceName);
extern "C" __declspec(dllexport) BOOL CanServiceBeStopped(const char* serviceName);