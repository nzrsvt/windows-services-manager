#include "pch.h"
#include "servicecontrol.h"
#include <windows.h>
#include <winsvc.h>
#include <iostream>

extern "C" __declspec(dllexport) void EnumerateServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    DWORD bytesNeeded, servicesCount, resumeHandle = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, &resumeHandle);

    if (GetLastError() != ERROR_MORE_DATA) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, &resumeHandle)) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        HeapFree(GetProcessHeap(), 0, services);
        return;
    }

    for (DWORD i = 0; i < servicesCount; i++) {
        std::wcout << L"Service name: " << services[i].lpServiceName << std::endl;
    }

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);
}

extern "C" __declspec(dllexport) void StartServiceC(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_START);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }


    if (!StartService(service, 0, NULL)) {
        std::cerr << "Failed to start service." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service started successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}

extern "C" __declspec(dllexport) void StopService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_STOP);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus)) {
        std::cerr << "Failed to stop service." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service stopped successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}


extern "C" __declspec(dllexport) void RestartService(const char* serviceName) {
    StopService(serviceName);

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_START);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    if (!StartService(service, 0, NULL)) {
        std::cerr << "Failed to start service." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service restarted successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}

extern "C" __declspec(dllexport) void PauseService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_PAUSE_CONTINUE);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_PAUSE, &serviceStatus)) {
        std::cerr << "Failed to pause service." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service paused successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}

extern "C" __declspec(dllexport) void ContinueService(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_PAUSE_CONTINUE);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(service, SERVICE_CONTROL_CONTINUE, &serviceStatus)) {
        std::cerr << "Failed to continue service." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service resumed successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}

extern "C" __declspec(dllexport) void ChangeStartType(const char* serviceName, DWORD startType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_CHANGE_CONFIG);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    if (!ChangeServiceConfig(service, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
        std::cerr << "Failed to change service start type." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    std::cout << "Service start type changed successfully." << std::endl;

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}

extern "C" __declspec(dllexport) void FindServiceByName(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    DWORD bytesNeeded, servicesCount, resumeHandle = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, &resumeHandle);

    if (GetLastError() != ERROR_MORE_DATA) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, &resumeHandle)) {
        std::cerr << "Failed to enumerate services." << std::endl;
        CloseServiceHandle(scm);
        HeapFree(GetProcessHeap(), 0, services);
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    bool found = false;
    for (DWORD i = 0; i < servicesCount; i++) {
        if (wcscmp(services[i].lpServiceName, wideServiceName) == 0) {
            std::cout << "Service found: " << services[i].lpServiceName << std::endl;
            found = true;
            break;
        }
    }

    delete[] wideServiceName; 

    if (!found) {
        std::cout << "Service not found." << std::endl;
    }

    CloseServiceHandle(scm);
    HeapFree(GetProcessHeap(), 0, services);
}

extern "C" __declspec(dllexport) void GetServiceInfo(const char* serviceName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        std::cerr << "Failed to open service control manager." << std::endl;
        return;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, serviceName, -1, NULL, 0);
    wchar_t* wideServiceName = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, serviceName, -1, wideServiceName, len);

    SC_HANDLE service = OpenService(scm, wideServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

    delete[] wideServiceName;

    if (service == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(scm);
        return;
    }

    SERVICE_STATUS_PROCESS serviceStatus;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        std::cerr << "Failed to query service status." << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return;
    }

    LPQUERY_SERVICE_CONFIG serviceConfig = NULL;
    DWORD bufferSize = 0;
    if (!QueryServiceConfig(service, NULL, 0, &bufferSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        serviceConfig = (LPQUERY_SERVICE_CONFIG)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
        if (serviceConfig == NULL) {
            std::cerr << "Failed to allocate memory." << std::endl;
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return;
        }

        if (!QueryServiceConfig(service, serviceConfig, bufferSize, &bytesNeeded)) {
            std::cerr << "Failed to query service configuration." << std::endl;
            HeapFree(GetProcessHeap(), 0, serviceConfig);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return;
        }
    }

    std::cout << "Service Name: " << serviceName << std::endl;
    std::cout << "Service State: ";
    switch (serviceStatus.dwCurrentState) {
    case SERVICE_STOPPED:
        std::cout << "Stopped" << std::endl;
        break;
    case SERVICE_START_PENDING:
        std::cout << "Start Pending" << std::endl;
        break;
    case SERVICE_STOP_PENDING:
        std::cout << "Stop Pending" << std::endl;
        break;
    case SERVICE_RUNNING:
        std::cout << "Running" << std::endl;
        break;
    case SERVICE_CONTINUE_PENDING:
        std::cout << "Continue Pending" << std::endl;
        break;
    case SERVICE_PAUSE_PENDING:
        std::cout << "Pause Pending" << std::endl;
        break;
    case SERVICE_PAUSED:
        std::cout << "Paused" << std::endl;
        break;
    default:
        std::cout << "Unknown" << std::endl;
        break;
    }

    if (serviceConfig != NULL) {
        std::cout << "Service Start Type: ";
        switch (serviceConfig->dwStartType) {
        case SERVICE_BOOT_START:
            std::cout << "Boot" << std::endl;
            break;
        case SERVICE_SYSTEM_START:
            std::cout << "System" << std::endl;
            break;
        case SERVICE_AUTO_START:
            std::cout << "Auto" << std::endl;
            break;
        case SERVICE_DEMAND_START:
            std::cout << "Manual" << std::endl;
            break;
        case SERVICE_DISABLED:
            std::cout << "Disabled" << std::endl;
            break;
        default:
            std::cout << "Unknown" << std::endl;
            break;
        }

        HeapFree(GetProcessHeap(), 0, serviceConfig);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
}