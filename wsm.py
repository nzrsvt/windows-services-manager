from time import sleep
import tkinter as tk
from tkinter import ttk
import ctypes
from ctypes import wintypes
import os
from PIL import Image, ImageTk

current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir + "\\wsm_dll\\x64\\Debug\\", "wsm_dll.dll")
wsm_dll = ctypes.CDLL(dll_path)

wsm_dll.GetServicesCount.restype = wintypes.DWORD
wsm_dll.EnumerateServicesWithInfo.restype = ctypes.POINTER(ctypes.c_char_p)
wsm_dll.GetServiceInfo.argtypes = [ctypes.c_char_p]
wsm_dll.GetServiceInfo.restype = ctypes.c_char_p
wsm_dll.CanServiceBePaused.argtypes = [ctypes.c_char_p]
wsm_dll.CanServiceBePaused.restype = ctypes.c_bool

SERVICE_AUTO_START = 2
SERVICE_BOOT_START = 0
SERVICE_DEMAND_START = 3
SERVICE_DISABLED = 4
SERVICE_SYSTEM_START = 1

start_image = Image.open("icons/start_icon.png").resize((32, 32), Image.LANCZOS)
stop_image = Image.open("icons/stop_icon.png").resize((32, 32), Image.LANCZOS)
restart_image = Image.open("icons/restart_icon.png").resize((32, 32), Image.LANCZOS)
pause_image = Image.open("icons/pause_icon.png").resize((32, 32), Image.LANCZOS)
continue_image = Image.open("icons/continue_icon.png").resize((32, 32), Image.LANCZOS)

def update_buttons_state(event=None):
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        current_state = wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[0]

        if current_state == "Stopped":
            start_button.config(state=tk.NORMAL)
            stop_button.config(state=tk.DISABLED)
            restart_button.config(state=tk.DISABLED)
            pause_button.config(state=tk.DISABLED)
            continue_button.config(state=tk.DISABLED)
            change_start_type_button.config(state=tk.NORMAL)
        elif current_state == "Running":
            start_button.config(state=tk.DISABLED)
            stop_button.config(state=tk.NORMAL)
            restart_button.config(state=tk.NORMAL)
            pause_button.config(state=tk.NORMAL)
            continue_button.config(state=tk.DISABLED)
            change_start_type_button.config(state=tk.NORMAL)
        elif current_state == "Paused":
            start_button.config(state=tk.DISABLED)
            stop_button.config(state=tk.DISABLED)
            restart_button.config(state=tk.DISABLED)
            pause_button.config(state=tk.DISABLED)
            continue_button.config(state=tk.NORMAL)
            change_start_type_button.config(state=tk.NORMAL)
        elif current_state in ["Start Pending", "Stop Pending", "Continue Pending", "Pause Pending"]:
            start_button.config(state=tk.DISABLED)
            stop_button.config(state=tk.DISABLED)
            restart_button.config(state=tk.DISABLED)
            pause_button.config(state=tk.DISABLED)
            continue_button.config(state=tk.DISABLED)
            change_start_type_button.config(state=tk.NORMAL)
        else: 
            start_button.config(state=tk.DISABLED)
            stop_button.config(state=tk.DISABLED)
            restart_button.config(state=tk.DISABLED)
            pause_button.config(state=tk.DISABLED)
            continue_button.config(state=tk.DISABLED)
            change_start_type_button.config(state=tk.DISABLED)
    else:
        start_button.config(state=tk.DISABLED)
        stop_button.config(state=tk.DISABLED)
        restart_button.config(state=tk.DISABLED)
        pause_button.config(state=tk.DISABLED)
        continue_button.config(state=tk.DISABLED)
        change_start_type_button.config(state=tk.DISABLED)

def wait_until_service_state(service_name, state):
    current_state = wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[0]

    if current_state == state:
        update_services_tree()
        return
    else:
        if state == "Running":
            if current_state == "Stopped":
                update_services_tree()
                status_label.config(text=f"Service '{service_name}' has stopped unexpectedly.")
                return
        elif state == "Stopped":
            if current_state == "Running":
                update_services_tree()
                status_label.config(text=f"Service '{service_name}' has started unexpectedly.")
                return
        root.after(1000, wait_until_service_state, service_name, state)

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
            wait_until_service_state(service_name, "Running")
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
            wait_until_service_state(service_name, "Stopped")
        else:
            status_label.config(text=f"Failed to stop service '{service_name}'.")

def restart_service():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        result = wsm_dll.RestartService(service_name.encode('utf-8'))
        if result == 0:
            status_label.config(text=f"Service '{service_name}' restarted successfully.")
            wait_until_service_state(service_name, "Running")
        else:
            status_label.config(text=f"Failed to restart service '{service_name}'.")
        update_services_tree()

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

def change_start_type(service_name, start_type):
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
    
    update_buttons_state()

def on_change_start_type_button_click():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        new_start_type = show_start_type_selection_dialog(service_name)
        if new_start_type is not None:
            change_start_type(service_name, new_start_type)

def show_start_type_selection_dialog(service_name):
    dialog = tk.Toplevel(root)
    dialog.title("Select Start Type")

    current_start_type = wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[1]

    start_type_mapping = {
        "Boot": SERVICE_BOOT_START,
        "System": SERVICE_SYSTEM_START,
        "Auto": SERVICE_AUTO_START,
        "Manual": SERVICE_DEMAND_START,
        "Disabled": SERVICE_DISABLED,
        "Unknown": 0 
    }

    selected_start_type = tk.IntVar(value=start_type_mapping.get(current_start_type, 0))  

    ttk.Radiobutton(dialog, text="Auto Start", variable=selected_start_type, value=SERVICE_AUTO_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Boot Start", variable=selected_start_type, value=SERVICE_BOOT_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Manual Start", variable=selected_start_type, value=SERVICE_DEMAND_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Disabled", variable=selected_start_type, value=SERVICE_DISABLED).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="System Start", variable=selected_start_type, value=SERVICE_SYSTEM_START).pack(anchor=tk.W)

    confirm_button = ttk.Button(dialog, text="Confirm", command=lambda: dialog.destroy())
    confirm_button.pack()

    dialog.wait_window()

    return selected_start_type.get()

root = tk.Tk()
root.title("Windows Services Manager")

search_frame = ttk.Frame(root)
search_frame.grid(row=0, column=0, sticky="ew")

search_label = ttk.Label(search_frame, text="Search Service:")
search_label.grid(row=0, column=0, padx=5, pady=5)

search_entry = ttk.Entry(search_frame, width=30)
search_entry.grid(row=0, column=1, padx=5, pady=5)

search_button = ttk.Button(search_frame, text="Search", command=update_services_tree)
search_button.grid(row=0, column=2, padx=5, pady=5)

operations_frame = ttk.Frame(root)
operations_frame.grid(row=1, column=0, sticky="ew")

start_icon = ImageTk.PhotoImage(start_image)
stop_icon = ImageTk.PhotoImage(stop_image)
restart_icon = ImageTk.PhotoImage(restart_image)
pause_icon = ImageTk.PhotoImage(pause_image)
continue_icon = ImageTk.PhotoImage(continue_image)

start_button = ttk.Button(operations_frame, image=start_icon, command=start_service)
start_button.grid(row=0, column=0, padx=5, pady=5)

stop_button = ttk.Button(operations_frame, image=stop_icon, command=stop_service)
stop_button.grid(row=0, column=1, padx=5, pady=5)

restart_button = ttk.Button(operations_frame, image=restart_icon, command=restart_service)
restart_button.grid(row=0, column=2, padx=5, pady=5)

pause_button = ttk.Button(operations_frame, image=pause_icon, command=pause_service)
pause_button.grid(row=0, column=3, padx=5, pady=5)

continue_button = ttk.Button(operations_frame, image=continue_icon, command=continue_service)
continue_button.grid(row=0, column=4, padx=5, pady=5)

change_start_type_button = ttk.Button(operations_frame, text="Change Start Type", command=on_change_start_type_button_click)
change_start_type_button.grid(row=0, column=5, padx=5, pady=5)

services_frame = ttk.Frame(root)
services_frame.grid(row=2, column=0, sticky="nsew")

services_tree = ttk.Treeview(services_frame, columns=('Service Name', 'Service State', 'Service Start Type'), show='headings')
services_tree.heading('Service Name', text='Service Name')
services_tree.heading('Service State', text='Service State')
services_tree.heading('Service Start Type', text='Service Start Type')
services_tree.pack(fill='both', expand=True)

services_tree.bind('<<TreeviewSelect>>', update_buttons_state)

update_services_tree()

status_label = ttk.Label(root, text="", anchor='center')
status_label.grid(row=3, column=0, sticky="ew", padx=5, pady=5)

root.mainloop()