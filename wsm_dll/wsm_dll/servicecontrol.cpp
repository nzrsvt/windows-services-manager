#include "pch.h"
#include "servicecontrol.h"
#include <windows.h>
#include <winsvc.h>
#include <iostream>
#include <vector>
#include <string>

extern "C" __declspec(dllexport) const wchar_t** EnumerateServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return nullptr;
    }

    DWORD bytesNeeded, servicesCount, resumeHandle = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, &resumeHandle);

    if (GetLastError() != ERROR_MORE_DATA) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        return nullptr;
    }

    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, &resumeHandle)) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        HeapFree(GetProcessHeap(), 0, services);
        return nullptr;
    }

    // Allocate memory for array of wchar_t pointers
    const wchar_t** serviceNames = new const wchar_t* [servicesCount];

    for (DWORD i = 0; i < servicesCount; i++) {
        // Allocate memory for each service name and copy it
        size_t len = wcslen(services[i].lpServiceName);
        serviceNames[i] = new wchar_t[len + 1];
        wcscpy_s(const_cast<wchar_t*>(serviceNames[i]), len + 1, services[i].lpServiceName);
    }

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);

    // Return pointer to array of service names
    return serviceNames;
}

extern "C" __declspec(dllexport) int StartServiceC(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_START);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }


    if (!StartService(service, 0, NULL)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to start service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service started successfully
}

extern "C" __declspec(dllexport) int StopService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_STOP);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to stop service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service stopped successfully
}

extern "C" __declspec(dllexport) int RestartService(const char* serviceName) {
    int stopResult = StopService(serviceName);
    if (stopResult != 0) {
        return stopResult; // Return the result of stopping the service
    }

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_START);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    if (!StartService(service, 0, NULL)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to start service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service restarted successfully
}

extern "C" __declspec(dllexport) int PauseService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_PAUSE_CONTINUE);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_PAUSE, &serviceStatus)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to pause service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service paused successfully
}

extern "C" __declspec(dllexport) int ContinueService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_PAUSE_CONTINUE);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_CONTINUE, &serviceStatus)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to continue service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service resumed successfully
}

extern "C" __declspec(dllexport) int ChangeStartType(const char* serviceName, DWORD startType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_CHANGE_CONFIG);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    if (!ChangeServiceConfig(service, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to change service start type
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service start type changed successfully
}

extern "C" __declspec(dllexport) const wchar_t* FindServiceByName(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scm == NULL) {
        return L"Failed to open service control manager.";
    }

    DWORD bytesNeeded, servicesCount, resumeHandle = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, &resumeHandle);

    if (GetLastError() != ERROR_MORE_DATA) {
        CloseServiceHandle(scm);
        return L"Failed to enumerate services.";
    }

    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, &resumeHandle)) {
        CloseServiceHandle(scm);
        HeapFree(GetProcessHeap(), 0, services);
        return L"Failed to enumerate services.";
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    const wchar_t* result = L"Service not found.";
    for (DWORD i = 0; i < servicesCount; i++) {
        if (wcscmp(services[i].lpServiceName, wideServiceName) == 0) {
            result = services[i].lpServiceName;
            break;
        }
    }

    delete[] wideServiceName;

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);
    return result;
}

extern "C" __declspec(dllexport) const wchar_t* GetServiceInfo(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        return L"Failed to open service control manager.";
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

    if (service == NULL) {
        CloseServiceHandle(scm);
        return L"Failed to open service.";
    }

    SERVICE_STATUS_PROCESS serviceStatus;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return L"Failed to query service status.";
    }

    LPQUERY_SERVICE_CONFIG serviceConfig = NULL;
    DWORD bufferSize = 0;
    if (!QueryServiceConfig(service, NULL, 0, &bufferSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        serviceConfig = (LPQUERY_SERVICE_CONFIG)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
        if (serviceConfig == NULL) {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return L"Failed to allocate memory.";
        }

        if (!QueryServiceConfig(service, serviceConfig, bufferSize, &bytesNeeded)) {
            HeapFree(GetProcessHeap(), 0, serviceConfig);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return L"Failed to query service configuration.";
        }
    }

    std::wstring serviceInfo;

    serviceInfo += L"Service Name: ";
    serviceInfo += wideServiceName;
    serviceInfo += L"\n";

    delete[] wideServiceName;

    serviceInfo += L"Service State: ";
    switch (serviceStatus.dwCurrentState) {
    case SERVICE_STOPPED:
        serviceInfo += L"Stopped\n";
        break;
    case SERVICE_START_PENDING:
        serviceInfo += L"Start Pending\n";
        break;
    case SERVICE_STOP_PENDING:
        serviceInfo += L"Stop Pending\n";
        break;
    case SERVICE_RUNNING:
        serviceInfo += L"Running\n";
        break;
    case SERVICE_CONTINUE_PENDING:
        serviceInfo += L"Continue Pending\n";
        break;
    case SERVICE_PAUSE_PENDING:
        serviceInfo += L"Pause Pending\n";
        break;
    case SERVICE_PAUSED:
        serviceInfo += L"Paused\n";
        break;
    default:
        serviceInfo += L"Unknown\n";
        break;
    }

    if (serviceConfig != NULL) {
        serviceInfo += L"Service Start Type: ";
        switch (serviceConfig->dwStartType) {
        case SERVICE_BOOT_START:
            serviceInfo += L"Boot\n";
            break;
        case SERVICE_SYSTEM_START:
            serviceInfo += L"System\n";
            break;
        case SERVICE_AUTO_START:
            serviceInfo += L"Auto\n";
            break;
        case SERVICE_DEMAND_START:
            serviceInfo += L"Manual\n";
            break;
        case SERVICE_DISABLED:
            serviceInfo += L"Disabled\n";
            break;
        default:
            serviceInfo += L"Unknown\n";
            break;
        }

        HeapFree(GetProcessHeap(), 0, serviceConfig);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    // Convert std::wstring to const wchar_t* for return
    const wchar_t* result = serviceInfo.c_str();
    return result;
}