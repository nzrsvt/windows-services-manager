#pragma once

extern "C" __declspec(dllexport) void EnumerateServices();
extern "C" __declspec(dllexport) void StartServiceC(const char* serviceName);
extern "C" __declspec(dllexport) void StopService(const char* serviceName);
extern "C" __declspec(dllexport) void RestartService(const char* serviceName);
extern "C" __declspec(dllexport) void PauseService(const char* serviceName);
extern "C" __declspec(dllexport) void ContinueService(const char* serviceName);
extern "C" __declspec(dllexport) void ChangeStartType(const char* serviceName, DWORD startType);
extern "C" __declspec(dllexport) void FindServiceByName(const char* serviceName);
extern "C" __declspec(dllexport) void GetServiceInfo(const char* serviceName);