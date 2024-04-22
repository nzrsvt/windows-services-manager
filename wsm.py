import tkinter as tk
from tkinter import ttk
import ctypes
from ctypes import wintypes
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir + "\\wsm_dll\\x64\\Debug\\", "wsm_dll.dll")
wsm_dll = ctypes.CDLL(dll_path)

wsm_dll.EnumerateServices.restype = ctypes.POINTER(ctypes.c_wchar_p)
wsm_dll.GetServicesCount.restype = wintypes.DWORD
wsm_dll.GetServiceInfo.argtypes = [ctypes.c_char_p]
wsm_dll.GetServiceInfo.restype = ctypes.c_char_p
wsm_dll.EnumerateServicesWithInfo.restype = ctypes.POINTER(ctypes.c_char_p)
wsm_dll.FindServiceByName.argtypes = [ctypes.c_char_p]
wsm_dll.FindServiceByName.restype = ctypes.POINTER(ctypes.c_wchar_p)

def get_services():
    services_ptr = wsm_dll.EnumerateServicesWithInfo()
    services = []

    if wsm_dll.EnumerateServicesWithInfo():
        for i in range(wsm_dll.GetServicesCount()):
            services.append(services_ptr[i].decode("utf-8"))
    
    return services

def start_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.StartServiceC(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' started successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to start service '{service_name}'.")

def stop_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.StopService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' stopped successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to stop service '{service_name}'.")

def restart_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.RestartService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' restarted successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to restart service '{service_name}'.")

def pause_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.PauseService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' paused successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to pause service '{service_name}'.")

def continue_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.ContinueService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' resumed successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to resume service '{service_name}'.")

def change_start_type():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        start_type = SERVICE_AUTO_START 
        result = wsm_dll.ChangeStartType(service_name.encode('utf-8'), start_type)
        if result == 0:
            status_label.config(text=f"Start type of service '{service_name}' changed successfully.")
            update_services_tree()
        else:
            status_label.config(text=f"Failed to change start type of service '{service_name}'.")

def update_services_tree(services=None):
    if services_tree.get_children():
        for item in services_tree.get_children():
            services_tree.delete(item)

    services = get_services()

    search_string = search_entry.get()
    if search_string:
        services = [service for service in services if search_string.lower() in service.split(',')[0].lower()]

    for service in services:
        service_name, service_state, service_start_type = service.split(',')
        services_tree.insert('', 'end', text=service_name, values=(service_name, service_state, service_start_type))


root = tk.Tk()
root.title("Windows Services Manager")

search_frame = ttk.Frame(root)
search_frame.pack(fill='x')

search_label = ttk.Label(search_frame, text="Search Service:")
search_label.grid(row=0, column=0, padx=5, pady=5)

search_entry = ttk.Entry(search_frame, width=30)
search_entry.grid(row=0, column=1, padx=5, pady=5)

search_button = ttk.Button(search_frame, text="Search", command=update_services_tree)
search_button.grid(row=0, column=2, padx=5, pady=5)

services_frame = ttk.Frame(root)
services_frame.pack(fill='both', expand=True)

services_tree = ttk.Treeview(services_frame, columns=('Service Name', 'Service State', 'Service Start Type'), show='headings')
services_tree.heading('Service Name', text='Service Name')
services_tree.heading('Service State', text='Service State')
services_tree.heading('Service Start Type', text='Service Start Type')
services_tree.pack(fill='both', expand=True)

update_services_tree()

operations_frame = ttk.Frame(root)
operations_frame.pack()

start_button = ttk.Button(operations_frame, text="Start Service", command=start_service)
start_button.grid(row=0, column=0, padx=5, pady=5)

stop_button = ttk.Button(operations_frame, text="Stop Service", command=stop_service)
stop_button.grid(row=0, column=1, padx=5, pady=5)

restart_button = ttk.Button(operations_frame, text="Restart Service", command=restart_service)
restart_button.grid(row=0, column=2, padx=5, pady=5)

pause_button = ttk.Button(operations_frame, text="Pause Service", command=pause_service)
pause_button.grid(row=0, column=3, padx=5, pady=5)

continue_button = ttk.Button(operations_frame, text="Continue Service", command=continue_service)
continue_button.grid(row=0, column=4, padx=5, pady=5)

change_start_type_button = ttk.Button(operations_frame, text="Change Start Type", command=change_start_type)
change_start_type_button.grid(row=0, column=5, padx=5, pady=5)

status_label = ttk.Label(root, text="")
status_label.pack()

root.mainloop()