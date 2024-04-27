from time import sleep
import tkinter as tk
from tkinter import ttk
import ctypes
from ctypes import wintypes
import os
from PIL import Image, ImageTk
from idlelib.tooltip import Hovertip

import sv_ttk

current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir + "\\wsm_dll\\x64\\Debug\\", "wsm_dll.dll")
wsm_dll = ctypes.CDLL(dll_path)

wsm_dll.GetServicesCount.restype = wintypes.DWORD
wsm_dll.EnumerateServicesWithInfo.restype = ctypes.POINTER(ctypes.c_char_p)
wsm_dll.GetServiceInfo.argtypes = [ctypes.c_char_p]
wsm_dll.GetServiceInfo.restype = ctypes.c_char_p
wsm_dll.CanServiceBePaused.argtypes = [ctypes.c_char_p]
wsm_dll.CanServiceBePaused.restype = ctypes.c_bool
wsm_dll.CanServiceBeStopped.argtypes = [ctypes.c_char_p]
wsm_dll.CanServiceBeStopped.restype = ctypes.c_bool

SERVICE_AUTO_START = 2
SERVICE_BOOT_START = 0
SERVICE_DEMAND_START = 3
SERVICE_DISABLED = 4
SERVICE_SYSTEM_START = 1

start_type_mapping = {
    "Boot": SERVICE_BOOT_START,
    "System": SERVICE_SYSTEM_START,
    "Auto": SERVICE_AUTO_START,
    "Manual": SERVICE_DEMAND_START,
    "Disabled": SERVICE_DISABLED,
    "Unknown": 0 
}

search_image = Image.open("icons/search_icon.png").resize((32, 32), Image.LANCZOS)
start_image = Image.open("icons/start_icon.png").resize((32, 32), Image.LANCZOS)
stop_image = Image.open("icons/stop_icon.png").resize((32, 32), Image.LANCZOS)
restart_image = Image.open("icons/restart_icon.png").resize((32, 32), Image.LANCZOS)
pause_image = Image.open("icons/pause_icon.png").resize((32, 32), Image.LANCZOS)
continue_image = Image.open("icons/continue_icon.png").resize((32, 32), Image.LANCZOS)
change_start_type_image = Image.open("icons/change_start_type_icon.png").resize((32, 32), Image.LANCZOS)

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
            state = tk.DISABLED
            if wsm_dll.CanServiceBeStopped(service_name.encode("utf-8")):
                state = tk.NORMAL
            stop_button.config(state=state)
            restart_button.config(state=state)
            pause_button.config(state=tk.NORMAL if wsm_dll.CanServiceBePaused(service_name.encode("utf-8")) else tk.DISABLED)
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
        elif result == 0:
            status_label.config(text=f"Failed to restart service '{service_name}'.")
            update_services_tree()
        elif result == -2:
            status_label.config(text=f"Failed to restart service '{service_name}'.")
            wait_until_service_state(service_name, "Stopped")

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
    selected_service_name = None
    selected_service = services_tree.selection()
    if selected_service:
        selected_service_name = services_tree.item(selected_service, 'text')

    if services_tree.get_children():
        for item in services_tree.get_children():
            services_tree.delete(item)

    services = get_services()

    search_string = search_entry.get()
    if search_string != "Filter by name...":
        services = [service for service in services if search_string.lower() in service.split(',')[0].lower()]

    for service in services:
        service_name, service_state, service_start_type = service.split(',')
        services_tree.insert('', 'end', text=service_name, values=(service_name, service_state, service_start_type))
    
    if selected_service_name:
        for item in services_tree.get_children():
            if services_tree.item(item, 'text') == selected_service_name:
                services_tree.selection_set(item)
                services_tree.focus(item)
                break

    update_buttons_state()

def on_change_start_type_button_click():
    selected_service = services_tree.selection()
    if selected_service:
        service_name = services_tree.item(selected_service, 'text')
        current_start_type = wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[1]
        show_start_type_selection_dialog(service_name, current_start_type)

def show_start_type_selection_dialog(service_name, current_start_type):
    def confirm_and_change_start_type():
        new_start_type = selected_start_type.get()
        dialog.destroy()
        if new_start_type != start_type_mapping.get(current_start_type, 0):
            change_start_type(service_name, new_start_type)

    dialog = tk.Toplevel(root)
    dialog.title("Select Start Type")

    tk.Label(dialog, text="Choose new start type: ").pack()

    selected_start_type = tk.IntVar(value=start_type_mapping.get(current_start_type, 0))  

    ttk.Radiobutton(dialog, text="Auto Start", variable=selected_start_type, value=SERVICE_AUTO_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Boot Start", variable=selected_start_type, value=SERVICE_BOOT_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Manual Start", variable=selected_start_type, value=SERVICE_DEMAND_START).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="Disabled", variable=selected_start_type, value=SERVICE_DISABLED).pack(anchor=tk.W)
    ttk.Radiobutton(dialog, text="System Start", variable=selected_start_type, value=SERVICE_SYSTEM_START).pack(anchor=tk.W)

    confirm_button = ttk.Button(dialog, text="Confirm", command=confirm_and_change_start_type)
    confirm_button.pack()

    dialog.wait_window()

    return selected_start_type.get()

root = tk.Tk()
root.title("Windows Services Manager")

sv_ttk.set_theme("light")

operations_frame = ttk.Frame(root)
operations_frame.grid(row=0, column=0)

start_icon = ImageTk.PhotoImage(start_image)
stop_icon = ImageTk.PhotoImage(stop_image)
restart_icon = ImageTk.PhotoImage(restart_image)
pause_icon = ImageTk.PhotoImage(pause_image)
continue_icon = ImageTk.PhotoImage(continue_image)
change_start_type_icon = ImageTk.PhotoImage(change_start_type_image)

start_button = ttk.Button(operations_frame, image=start_icon, command=start_service)
start_button.grid(row=0, column=0, padx=5, pady=5)
start_tip = Hovertip(start_button,'Start Service', hover_delay=500)

stop_button = ttk.Button(operations_frame, image=stop_icon, command=stop_service)
stop_button.grid(row=0, column=1, padx=5, pady=5)
stop_tip = Hovertip(stop_button,'Stop Service', hover_delay=500)

restart_button = ttk.Button(operations_frame, image=restart_icon, command=restart_service)
restart_button.grid(row=0, column=2, padx=5, pady=5)
restart_tip = Hovertip(restart_button,'Restart Service', hover_delay=500)

pause_button = ttk.Button(operations_frame, image=pause_icon, command=pause_service)
pause_button.grid(row=0, column=3, padx=5, pady=5)
pause_tip = Hovertip(pause_button,'Pause Service', hover_delay=500)

continue_button = ttk.Button(operations_frame, image=continue_icon, command=continue_service)
continue_button.grid(row=0, column=4, padx=5, pady=5)
continue_tip = Hovertip(continue_button,'Continue Service', hover_delay=500)

change_start_type_button = ttk.Button(operations_frame, image=change_start_type_icon, command=on_change_start_type_button_click)
change_start_type_button.grid(row=0, column=5, padx=5, pady=5)
change_start_type_tip = Hovertip(change_start_type_button,'Change Start Type', hover_delay=500)

def on_search_entry_click(event):
    if search_entry.get() == "Filter by name...":
        search_entry.delete(0, "end")
        search_entry.insert(0, '')
        search_entry.config(foreground = 'black')

def on_search_focusout(event):
    if search_entry.get() == '':
        search_entry.insert(0, 'Filter by name...')
        search_entry.config(foreground = 'grey')

search_entry = ttk.Entry(operations_frame, width=30)
search_entry.grid(row=0, column=6, padx=5, pady=5)
search_entry.insert(0, 'Filter by name...')
search_entry.config(foreground = 'grey')
search_entry.bind('<FocusIn>', on_search_entry_click)
search_entry.bind('<FocusOut>', on_search_focusout)

search_icon = ImageTk.PhotoImage(search_image)

search_button = ttk.Button(operations_frame, image=search_icon, command=update_services_tree)
search_button.grid(row=0, column=7, padx=5, pady=5)
search_tip = Hovertip(search_button,'Search Service', hover_delay=500)

services_frame = ttk.Frame(root)
services_frame.grid(row=1, column=0, sticky="nsew", padx=5)

services_tree_scrollbar = tk.Scrollbar(services_frame)
services_tree_scrollbar.pack(side="right", fill="y")

services_tree = ttk.Treeview(services_frame, columns=('Service Name', 'Service State', 'Service Start Type'), show='headings', yscrollcommand=services_tree_scrollbar.set)

services_tree_scrollbar.config(command=services_tree.yview)

def sort_treeview_column(tv, col, reverse):
    data = [(tv.set(child, col), child) for child in tv.get_children('')]
    data.sort(reverse=reverse)

    for index, (val, child) in enumerate(data):
        tv.move(child, '', index)

    tv.heading(col, command=lambda: sort_treeview_column(tv, col, not reverse))

services_tree.heading('Service Name', text='Service Name', command=lambda: sort_treeview_column(services_tree, 'Service Name', False))
services_tree.heading('Service State', text='Service State', command=lambda: sort_treeview_column(services_tree, 'Service State', False))
services_tree.heading('Service Start Type', text='Service Start Type', command=lambda: sort_treeview_column(services_tree, 'Service Start Type', False))
services_tree.pack(fill='both', expand=True)

services_tree.bind('<<TreeviewSelect>>', update_buttons_state)

update_services_tree()

status_label = ttk.Label(root, text="", anchor='center')
status_label.grid(row=2, column=0, sticky="ew", padx=5, pady=5)

root.mainloop()