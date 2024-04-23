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
    std::cout << stopResult;
    if (stopResult != 0 && stopResult != -3) {
        return -1;
    }

    int startResult = StartServiceC(serviceName);
    std::cout << startResult;
    if (startResult != 0) {
        return -2;
    }

    return 0;
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

extern "C" __declspec(dllexport) const char* GetServiceInfo(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        return "Failed to open service control manager.";
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

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

    switch (serviceStatus.dwCurrentState) {
    case SERVICE_STOPPED:
        serviceInfo += "Stopped,";
        break;
    case SERVICE_START_PENDING:
        serviceInfo += "Start Pending,";
        break;
    case SERVICE_STOP_PENDING:
        serviceInfo += "Stop Pending,";
        break;
    case SERVICE_RUNNING:
        serviceInfo += "Running,";
        break;
    case SERVICE_CONTINUE_PENDING:
        serviceInfo += "Continue Pending,";
        break;
    case SERVICE_PAUSE_PENDING:
        serviceInfo += "Pause Pending,";
        break;
    case SERVICE_PAUSED:
        serviceInfo += "Paused,";
        break;
    default:
        serviceInfo += "Unknown,";
        break;
    }

    if (serviceConfig != NULL) {
        switch (serviceConfig->dwStartType) {
        case SERVICE_BOOT_START:
            serviceInfo += "Boot";
            break;
        case SERVICE_SYSTEM_START:
            serviceInfo += "System";
            break;
        case SERVICE_AUTO_START:
            serviceInfo += "Auto";
            break;
        case SERVICE_DEMAND_START:
            serviceInfo += "Manual";
            break;
        case SERVICE_DISABLED:
            serviceInfo += "Disabled";
            break;
        default:
            serviceInfo += "Unknown";
            break;
        }

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
    const char** serviceInfos = new const char* [servicesCount];
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

        switch (services[i].ServiceStatus.dwCurrentState) {
        case SERVICE_STOPPED:
            serviceInfo += "Stopped,";
            break;
        case SERVICE_START_PENDING:
            serviceInfo += "Start Pending,";
            break;
        case SERVICE_STOP_PENDING:
            serviceInfo += "Stop Pending,";
            break;
        case SERVICE_RUNNING:
            serviceInfo += "Running,";
            break;
        case SERVICE_CONTINUE_PENDING:
            serviceInfo += "Continue Pending,";
            break;
        case SERVICE_PAUSE_PENDING:
            serviceInfo += "Pause Pending,";
            break;
        case SERVICE_PAUSED:
            serviceInfo += "Paused,";
            break;
        default:
            serviceInfo += "Unknown,";
            break;
        }

        if (serviceConfig != NULL) {
            switch (serviceConfig->dwStartType) {
            case SERVICE_BOOT_START:
                serviceInfo += "Boot";
                break;
            case SERVICE_SYSTEM_START:
                serviceInfo += "System";
                break;
            case SERVICE_AUTO_START:
                serviceInfo += "Auto";
                break;
            case SERVICE_DEMAND_START:
                serviceInfo += "Manual";
                break;
            case SERVICE_DISABLED:
                serviceInfo += "Disabled";
                break;
            default:
                serviceInfo += "Unknown";
                break;
            }

            HeapFree(GetProcessHeap(), 0, serviceConfig);
        }

        // Allocate memory for each service info and copy it
        size_t len = serviceInfo.length();
        serviceInfos[i] = new char[len + 1];
        strcpy_s(const_cast<char*>(serviceInfos[i]), len + 1, serviceInfo.c_str());

        CloseServiceHandle(service);
    }

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);

    // Return pointer to array of service infos
    return serviceInfos;
}

extern "C" __declspec(dllexport) BOOL CanServiceBePaused(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return FALSE;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

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