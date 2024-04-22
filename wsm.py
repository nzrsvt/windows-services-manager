import tkinter as tk
from tkinter import ttk
import ctypes
from ctypes import wintypes
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir + "\\wsm_dll\\x64\\Debug\\", "wsm_dll.dll")
wsm_dll = ctypes.CDLL(dll_path)

# Define ctypes function prototypes
wsm_dll.EnumerateServices.restype = ctypes.POINTER(ctypes.c_wchar_p)
wsm_dll.GetServicesCount.restype = wintypes.DWORD
wsm_dll.FindServiceByName.argtypes = [ctypes.c_char_p]
wsm_dll.FindServiceByName.restype = ctypes.c_wchar_p
wsm_dll.GetServiceInfo.argtypes = [ctypes.c_char_p]
wsm_dll.GetServiceInfo.restype = ctypes.c_char_p
wsm_dll.EnumerateServicesWithInfo.restype = ctypes.POINTER(ctypes.c_char_p)

# Function to get all services
def get_services():
    services_ptr = wsm_dll.EnumerateServicesWithInfo()
    services = []

    if wsm_dll.EnumerateServicesWithInfo():
        for i in range(wsm_dll.GetServicesCount()):
            services.append(services_ptr[i].decode("utf-8"))
    
    return services

# Function to find a service by name
def find_service(service_name):
    # Call the FindServiceByName function from the DLL
    result = wsm_dll.FindServiceByName(service_name.encode('utf-8'))
    return result.decode('utf-8')

# Function to start a service
def start_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.StartServiceC(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' started successfully.")
        else:
            status_label.config(text=f"Failed to start service '{service_name}'.")

# Function to stop a service
def stop_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.StopService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' stopped successfully.")
        else:
            status_label.config(text=f"Failed to stop service '{service_name}'.")

# Function to restart a service
def restart_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.RestartService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' restarted successfully.")
        else:
            status_label.config(text=f"Failed to restart service '{service_name}'.")

# Function to pause a service
def pause_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.PauseService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' paused successfully.")
        else:
            status_label.config(text=f"Failed to pause service '{service_name}'.")

# Function to continue a service
def continue_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.ContinueService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' resumed successfully.")
        else:
            status_label.config(text=f"Failed to resume service '{service_name}'.")

# Function to change start type of a service
def change_start_type():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        # You need to define the start type value (e.g., SERVICE_AUTO_START)
        start_type = SERVICE_AUTO_START  # Replace with the actual start type value
        result = wsm_dll.ChangeStartType(service_name.encode('utf-8'), start_type)
        if result == 0:
            status_label.config(text=f"Start type of service '{service_name}' changed successfully.")
        else:
            status_label.config(text=f"Failed to change start type of service '{service_name}'.")

# Function to handle search button click
def search_service():
    service_name = search_entry.get()
    if service_name:
        result = find_service(service_name)
        if result:
            status_label.config(text=f"Service '{service_name}' found.")
            # Update the treeview to show the found service
            services_tree.delete(*services_tree.get_children())
            services_tree.insert('', 'end', text=result, values=(result,))
        else:
            status_label.config(text=f"Service '{service_name}' not found.")
    else:
        status_label.config(text="Please enter a service name.")

def get_service_info(service_name):
    result = wsm_dll.GetServiceInfo(service_name.encode('utf-8'))
    if result:
        service_info = result.decode('utf-8')
        return service_info
    else:
        return None


# Create the main window
root = tk.Tk()
root.title("Windows Services Manager")

# Create a frame for the search section
search_frame = ttk.Frame(root)
search_frame.pack(fill='x')

# Create a label and entry for searching services
search_label = ttk.Label(search_frame, text="Search Service:")
search_label.grid(row=0, column=0, padx=5, pady=5)

search_entry = ttk.Entry(search_frame, width=30)
search_entry.grid(row=0, column=1, padx=5, pady=5)

search_button = ttk.Button(search_frame, text="Search", command=search_service)
search_button.grid(row=0, column=2, padx=5, pady=5)

# Create a frame for the services section
services_frame = ttk.Frame(root)
services_frame.pack(fill='both', expand=True)

# Create a treeview to display services
services_tree = ttk.Treeview(services_frame, columns=('Service Name', 'Service State', 'Service Start Type'), show='headings')
services_tree.heading('Service Name', text='Service Name')
services_tree.heading('Service State', text='Service State')
services_tree.heading('Service Start Type', text='Service Start Type')
services_tree.pack(fill='both', expand=True)

# Populate the treeview with services
services = get_services()

for service in services:
    service_name, service_state, service_start_type = service.split(',')
    services_tree.insert('', 'end', text=service_name, values=(service_name, service_state, service_start_type))

# Define columns for state and start type


# Create buttons for service operations
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

# Create a status label
status_label = ttk.Label(root, text="")
status_label.pack()

root.mainloop()