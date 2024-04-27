#include "pch.h"
#include "servicecontrol.h"
#include <windows.h>
#include <iostream>

wchar_t* ConvertToWideString(const char* str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (len == 0) {
        return NULL;
    }
    wchar_t* wideStr = new wchar_t[len];
    if (MultiByteToWideChar(CP_ACP, 0, str, -1, wideStr, len) == 0) {
        delete[] wideStr;
        return NULL;
    }
    return wideStr;
}

std::string GetServiceStateString(DWORD currentState) {
    std::string stateString;
    switch (currentState) {
    case SERVICE_STOPPED:
        stateString = "Stopped";
        break;
    case SERVICE_START_PENDING:
        stateString = "Start Pending";
        break;
    case SERVICE_STOP_PENDING:
        stateString = "Stop Pending";
        break;
    case SERVICE_RUNNING:
        stateString = "Running";
        break;
    case SERVICE_CONTINUE_PENDING:
        stateString = "Continue Pending";
        break;
    case SERVICE_PAUSE_PENDING:
        stateString = "Pause Pending";
        break;
    case SERVICE_PAUSED:
        stateString = "Paused";
        break;
    default:
        stateString = "Unknown";
        break;
    }
    return stateString;
}

std::string GetStartTypeString(DWORD startType) {
    std::string startTypeString;
    switch (startType) {
    case SERVICE_BOOT_START:
        startTypeString = "Boot";
        break;
    case SERVICE_SYSTEM_START:
        startTypeString = "System";
        break;
    case SERVICE_AUTO_START:
        startTypeString = "Auto";
        break;
    case SERVICE_DEMAND_START:
        startTypeString = "Manual";
        break;
    case SERVICE_DISABLED:
        startTypeString = "Disabled";
        break;
    default:
        startTypeString = "Unknown";
        break;
    }
    return startTypeString;
}

int PerformServiceControl(const char* serviceName, DWORD managerDesiredAccess, DWORD serviceDesiredAccess, DWORD controlType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, managerDesiredAccess);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

    SC_HANDLE service = OpenService(scm, wideServiceName, serviceDesiredAccess);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return -2; // Failed to open service
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, controlType, &serviceStatus)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return -3; // Failed to control service
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0; // Service controlled successfully
}

extern "C" __declspec(dllexport) int StartServiceC(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

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
    return PerformServiceControl(serviceName, SC_MANAGER_ALL_ACCESS, SERVICE_STOP, SERVICE_CONTROL_STOP);
}

extern "C" __declspec(dllexport) int PauseService(const char* serviceName) {
    return PerformServiceControl(serviceName, SC_MANAGER_ALL_ACCESS, SERVICE_PAUSE_CONTINUE, SERVICE_CONTROL_PAUSE);
}

extern "C" __declspec(dllexport) int ContinueService(const char* serviceName) {
    return PerformServiceControl(serviceName, SC_MANAGER_ALL_ACCESS, SERVICE_PAUSE_CONTINUE, SERVICE_CONTROL_CONTINUE);
}

extern "C" __declspec(dllexport) int ChangeStartType(const char* serviceName, DWORD startType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        return -1; // Failed to open service control manager
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

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

extern "C" __declspec(dllexport) const char* GetServiceInfo(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        return "Failed to open service control manager.";
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

    delete[] wideServiceName;

    if (service == NULL) {
        CloseServiceHandle(scm);
        return "Failed to open service.";
    }

    SERVICE_STATUS_PROCESS serviceStatus;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return "Failed to query service status.";
    }

    LPQUERY_SERVICE_CONFIG serviceConfig = NULL;
    DWORD bufferSize = 0;
    if (!QueryServiceConfig(service, NULL, 0, &bufferSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        serviceConfig = (LPQUERY_SERVICE_CONFIG)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
        if (serviceConfig == NULL) {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return "Failed to allocate memory.";
        }

        if (!QueryServiceConfig(service, serviceConfig, bufferSize, &bytesNeeded)) {
            HeapFree(GetProcessHeap(), 0, serviceConfig);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return "Failed to query service configuration.";
        }
    }

    std::string serviceInfo;

    serviceInfo += GetServiceStateString(serviceStatus.dwCurrentState);
    serviceInfo += ",";

    if (serviceConfig != NULL) {
        serviceInfo += GetStartTypeString(serviceConfig->dwStartType);

        HeapFree(GetProcessHeap(), 0, serviceConfig);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    // Convert std::string to const char* for return
    char* result = new char[serviceInfo.length() + 1];
    strcpy_s(result, serviceInfo.length() + 1, serviceInfo.c_str());

    return result;
}

extern "C" __declspec(dllexport) DWORD GetServicesCount() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return 0;
    }

    DWORD bytesNeeded, servicesCount, resumeHandle = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, &resumeHandle);

    if (GetLastError() != ERROR_MORE_DATA) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        return 0;
    }

    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, &resumeHandle)) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        HeapFree(GetProcessHeap(), 0, services);
        return 0;
    }
    CloseServiceHandle(scm);
    return servicesCount;
}

extern "C" __declspec(dllexport) const char** EnumerateServicesWithInfo() {
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

    // Allocate memory for array of char pointers
    const char** servicesInfo = new const char* [servicesCount];
    for (DWORD i = 0; i < servicesCount; i++) {
        SC_HANDLE service = OpenService(scm, services[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (service == NULL) {
            std::cerr << "Failed to open service." << std::endl;
            continue;
        }

        LPQUERY_SERVICE_CONFIG serviceConfig = NULL;
        DWORD bufferSize = 0;
        if (!QueryServiceConfig(service, NULL, 0, &bufferSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            serviceConfig = (LPQUERY_SERVICE_CONFIG)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
            if (serviceConfig == NULL) {
                std::cerr << "Failed to allocate memory." << std::endl;
                CloseServiceHandle(service);
                continue;
            }

            if (!QueryServiceConfig(service, serviceConfig, bufferSize, &bytesNeeded)) {
                std::cerr << "Failed to query service configuration." << std::endl;
                HeapFree(GetProcessHeap(), 0, serviceConfig);
                CloseServiceHandle(service);
                continue;
            }
        }

        int name_len = WideCharToMultiByte(CP_ACP, 0, services[i].lpServiceName, -1, NULL, 0, NULL, NULL);
        char* serviceNameBuf = new char[name_len];
        WideCharToMultiByte(CP_ACP, 0, services[i].lpServiceName, -1, serviceNameBuf, name_len, NULL, NULL);

        std::string serviceInfo = std::string(serviceNameBuf) + ",";

        delete[] serviceNameBuf;

        serviceInfo += GetServiceStateString(services[i].ServiceStatus.dwCurrentState);
        serviceInfo += ",";

        if (serviceConfig != NULL) {
            serviceInfo += GetStartTypeString(serviceConfig->dwStartType);

            HeapFree(GetProcessHeap(), 0, serviceConfig);
        }

        // Allocate memory for each service info and copy it
        size_t len = serviceInfo.length();
        servicesInfo[i] = new char[len + 1];
        strcpy_s(const_cast<char*>(servicesInfo[i]), len + 1, serviceInfo.c_str());

        CloseServiceHandle(service);
    }

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);

    // Return pointer to array of service info
    return servicesInfo;
}

extern "C" __declspec(dllexport) BOOL CanServiceBePaused(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return FALSE;
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_QUERY_STATUS);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return FALSE;
    }

    SERVICE_STATUS_PROCESS statusProcess;
    DWORD bytesNeeded;
    BOOL success = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);

    if (!success) {
        std::cerr << "Failed to query service status." << std::endl;
    }
    else {
        DWORD controlsAccepted = statusProcess.dwControlsAccepted;
        BOOL canBePaused = (controlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) != 0;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return canBePaused;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return FALSE;
}

extern "C" __declspec(dllexport) BOOL CanServiceBeStopped(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return FALSE;
    }

    wchar_t* wideServiceName = ConvertToWideString(serviceName);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_QUERY_STATUS);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return FALSE;
    }

    SERVICE_STATUS_PROCESS statusProcess;
    DWORD bytesNeeded;
    BOOL success = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);

    if (!success) {
        std::cerr << "Failed to query service status." << std::endl;
    }
    else {
        DWORD controlsAccepted = statusProcess.dwControlsAccepted;
        BOOL canBeStopped = (controlsAccepted & SERVICE_ACCEPT_STOP) != 0;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return canBeStopped;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return FALSE;
}