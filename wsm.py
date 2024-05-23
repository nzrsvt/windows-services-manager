import tkinter as tk
from tkinter import ttk
import ctypes
from ctypes import wintypes
import os
from PIL import Image, ImageTk
from idlelib.tooltip import Hovertip

import sv_ttk

class WindowsServiceManager:
    def __init__(self):
        self.setup_dll()
        self.setup_gui()
    
    def setup_dll(self):
        self.current_dir = os.path.dirname(os.path.abspath(__file__))
        self.dll_path = os.path.join(self.current_dir + "\\wsm_dll\\x64\\Debug\\", "wsm_dll.dll")
        self.wsm_dll = ctypes.CDLL(self.dll_path)

        self.wsm_dll.GetServicesCount.restype = wintypes.DWORD
        self.wsm_dll.EnumerateServicesWithInfo.restype = ctypes.POINTER(ctypes.c_char_p)
        self.wsm_dll.GetServiceInfo.argtypes = [ctypes.c_char_p]
        self.wsm_dll.GetServiceInfo.restype = ctypes.c_char_p
        self.wsm_dll.CanServiceBePaused.argtypes = [ctypes.c_char_p]
        self.wsm_dll.CanServiceBePaused.restype = ctypes.c_bool
        self.wsm_dll.CanServiceBeStopped.argtypes = [ctypes.c_char_p]
        self.wsm_dll.CanServiceBeStopped.restype = ctypes.c_bool

    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("Windows Services Manager")

        sv_ttk.set_theme("light")

        self.operations_frame = ttk.Frame(self.root)
        self.operations_frame.grid(row=0, column=0)
        
        self.icons = self.load_icons()
        self.setup_buttons()
        self.setup_search_entry()
        self.setup_services_tree()
        self.setup_status_label()

    def setup_buttons(self):
        self.start_button = ttk.Button(self.operations_frame, image=self.icons["start"], command=self.start_service)
        self.start_button.grid(row=0, column=0, padx=5, pady=5)
        Hovertip(self.start_button, 'Start Service', hover_delay=500)

        self.stop_button = ttk.Button(self.operations_frame, image=self.icons["stop"], command=self.stop_service)
        self.stop_button.grid(row=0, column=1, padx=5, pady=5)
        Hovertip(self.stop_button, 'Stop Service', hover_delay=500)

        self.restart_button = ttk.Button(self.operations_frame, image=self.icons["restart"], command=self.restart_service)
        self.restart_button.grid(row=0, column=2, padx=5, pady=5)
        Hovertip(self.restart_button, 'Restart Service', hover_delay=500)

        self.pause_button = ttk.Button(self.operations_frame, image=self.icons["pause"], command=self.pause_service)
        self.pause_button.grid(row=0, column=3, padx=5, pady=5)
        Hovertip(self.pause_button, 'Pause Service', hover_delay=500)

        self.continue_button = ttk.Button(self.operations_frame, image=self.icons["continue"], command=self.continue_service)
        self.continue_button.grid(row=0, column=4, padx=5, pady=5)
        Hovertip(self.continue_button, 'Continue Service', hover_delay=500)

        self.change_start_type_button = ttk.Button(self.operations_frame, image=self.icons["change_start_type"], command=self.on_change_start_type_button_click)
        self.change_start_type_button.grid(row=0, column=5, padx=5, pady=5)
        Hovertip(self.change_start_type_button, 'Change Start Type', hover_delay=500)

    def load_icons(self):
        icon_paths = {
            "start": "icons/start_icon.png",
            "stop": "icons/stop_icon.png",
            "restart": "icons/restart_icon.png",
            "pause": "icons/pause_icon.png",
            "continue": "icons/continue_icon.png",
            "change_start_type": "icons/change_start_type_icon.png",
            "search": "icons/search_icon.png"
        }
        icons = {}
        for name, path in icon_paths.items():
            image = Image.open(path).resize((32, 32), Image.LANCZOS)
            icons[name] = ImageTk.PhotoImage(image)
        return icons

    def setup_search_entry(self):
        self.search_entry = ttk.Entry(self.operations_frame, width=30)
        self.search_entry.grid(row=0, column=6, padx=5, pady=5)
        self.search_entry.insert(0, 'Filter by name...')
        self.search_entry.config(foreground='grey')
        self.search_entry.bind('<FocusIn>', self.on_search_entry_click)
        self.search_entry.bind('<FocusOut>', self.on_search_focusout)

        self.search_button = ttk.Button(self.operations_frame, image=self.icons["search"], command=self.update_services_tree)
        self.search_button.grid(row=0, column=7, padx=5, pady=5)
        Hovertip(self.search_button, 'Search Service', hover_delay=500)

    def setup_services_tree(self):
        self.current_sort_column = None
        self.current_sort_reverse = False

        self.services_frame = ttk.Frame(self.root)
        self.services_frame.grid(row=1, column=0, sticky="nsew", padx=5)
        self.services_tree_scrollbar = tk.Scrollbar(self.services_frame)
        self.services_tree_scrollbar.pack(side="right", fill="y")

        self.services_tree = ttk.Treeview(self.services_frame, columns=('Service Name', 'Service State', 'Service Start Type'), show='headings', yscrollcommand=self.services_tree_scrollbar.set)
        self.services_tree_scrollbar.config(command=self.services_tree.yview)

        self.services_tree.heading('Service Name', text='Service Name', command=lambda: self.sort_treeview_column('Service Name', False))
        self.services_tree.heading('Service State', text='Service State', command=lambda: self.sort_treeview_column('Service State', False))
        self.services_tree.heading('Service Start Type', text='Service Start Type', command=lambda: self.sort_treeview_column('Service Start Type', False))
        self.services_tree.pack(fill='both', expand=True)

        self.services_tree.bind('<<TreeviewSelect>>', self.update_buttons_state)
        self.update_services_tree()

    def sort_treeview_column(self, col, reverse):
        self.current_sort_column = col
        self.current_sort_reverse = reverse

        data = [(self.services_tree.set(child, col), child) for child in self.services_tree.get_children('')]
        data.sort(reverse=reverse)
        for index, (val, child) in enumerate(data):
            self.services_tree.move(child, '', index)
        self.services_tree.heading(col, command=lambda: self.sort_treeview_column(col, not reverse))

    def setup_status_label(self):
        self.status_label = ttk.Label(self.root, text="", anchor='center')
        self.status_label.grid(row=2, column=0, sticky="ew", padx=5, pady=5)

    def on_search_entry_click(self, event):
        if self.search_entry.get() == "Filter by name...":
            self.search_entry.delete(0, "end")
            self.search_entry.insert(0, '')
            self.search_entry.config(foreground='black')

    def on_search_focusout(self, event):
        if self.search_entry.get() == '':
            self.search_entry.insert(0, 'Filter by name...')
            self.search_entry.config(foreground='grey')

    def update_buttons_state(self, event=None):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            current_state = self.wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[0]

            if current_state == "Stopped":
                self.start_button.config(state=tk.NORMAL)
                self.stop_button.config(state=tk.DISABLED)
                self.restart_button.config(state=tk.DISABLED)
                self.pause_button.config(state=tk.DISABLED)
                self.continue_button.config(state=tk.DISABLED)
                self.change_start_type_button.config(state=tk.NORMAL)
            elif current_state == "Running":
                self.start_button.config(state=tk.DISABLED)
                state = tk.DISABLED
                if self.wsm_dll.CanServiceBeStopped(service_name.encode("utf-8")):
                    state = tk.NORMAL
                self.stop_button.config(state=state)
                self.restart_button.config(state=state)
                self.pause_button.config(state=tk.NORMAL if self.wsm_dll.CanServiceBePaused(service_name.encode("utf-8")) else tk.DISABLED)
                self.continue_button.config(state=tk.DISABLED)
                self.change_start_type_button.config(state=tk.NORMAL)
            elif current_state == "Paused":
                self.start_button.config(state=tk.DISABLED)
                self.stop_button.config(state=tk.DISABLED)
                self.restart_button.config(state=tk.DISABLED)
                self.pause_button.config(state=tk.DISABLED)
                self.continue_button.config(state=tk.NORMAL)
                self.change_start_type_button.config(state=tk.NORMAL)
            elif current_state in ["Start Pending", "Stop Pending", "Continue Pending", "Pause Pending"]:
                self.start_button.config(state=tk.DISABLED)
                self.stop_button.config(state=tk.DISABLED)
                self.restart_button.config(state=tk.DISABLED)
                self.pause_button.config(state=tk.DISABLED)
                self.continue_button.config(state=tk.DISABLED)
                self.change_start_type_button.config(state=tk.NORMAL)
            else:
                self.start_button.config(state=tk.DISABLED)
                self.stop_button.config(state=tk.DISABLED)
                self.restart_button.config(state=tk.DISABLED)
                self.pause_button.config(state=tk.DISABLED)
                self.continue_button.config(state=tk.DISABLED)
                self.change_start_type_button.config(state=tk.DISABLED)
        else:
            self.start_button.config(state=tk.DISABLED)
            self.stop_button.config(state=tk.DISABLED)
            self.restart_button.config(state=tk.DISABLED)
            self.pause_button.config(state=tk.DISABLED)
            self.continue_button.config(state=tk.DISABLED)
            self.change_start_type_button.config(state=tk.DISABLED)

    def start_service(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            result = self.wsm_dll.StartServiceC(service_name.encode('utf-8'))
            if result == 0:
                self.status_label.config(text=f"Service '{service_name}' started successfully.")
                self.update_services_tree()
                self.wait_until_service_state(service_name, "Running")
            else:
                self.status_label.config(text=f"Failed to start service '{service_name}'.")

    def stop_service(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            result = self.wsm_dll.StopService(service_name.encode('utf-8'))
            if result == 0:
                self.status_label.config(text=f"Service '{service_name}' stopped successfully.")
                self.update_services_tree()
                self.wait_until_service_state(service_name, "Stopped")
            else:
                self.status_label.config(text=f"Failed to stop service '{service_name}'.")

    def restart_service(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            self.stop_service()
            self.wait_until_service_state(service_name, "Stopped(Restart)")

    def pause_service(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            result = self.wsm_dll.PauseService(service_name.encode('utf-8'))
            if result == 0:
                self.status_label.config(text=f"Service '{service_name}' paused successfully.")
                self.update_services_tree()
            else:
                self.status_label.config(text=f"Failed to pause service '{service_name}'.")

    def continue_service(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            result = self.wsm_dll.ContinueService(service_name.encode('utf-8'))
            if result == 0:
                self.status_label.config(text=f"Service '{service_name}' resumed successfully.")
                self.update_services_tree()
            else:
                self.status_label.config(text=f"Failed to resume service '{service_name}'.")

    def change_start_type(self, service_name, start_type):
        result = self.wsm_dll.ChangeStartType(service_name.encode('utf-8'), start_type)
        if result == 0:
            self.status_label.config(text=f"Start type of service '{service_name}' changed successfully.")
            self.update_services_tree()
        else:
            self.status_label.config(text=f"Failed to change start type of service '{service_name}'.")

    def update_services_tree(self):
        selected_service_name = None
        selected_service = self.services_tree.selection()
        if selected_service:
            selected_service_name = self.services_tree.item(selected_service, 'text')

        if self.services_tree.get_children():
            for item in self.services_tree.get_children():
                self.services_tree.delete(item)

        services = self.get_services()

        search_string = self.search_entry.get()
        if search_string != "Filter by name...":
            services = [service for service in services if search_string.lower() in service.split(',')[0].lower()]

        for service in services:
            service_name, service_state, service_start_type = service.split(',')
            self.services_tree.insert('', 'end', text=service_name, values=(service_name, service_state, service_start_type))

        if selected_service_name:
            for item in self.services_tree.get_children():
                if self.services_tree.item(item, 'text') == selected_service_name:
                    self.services_tree.selection_set(item)
                    self.services_tree.focus(item)
                    break
        if self.current_sort_column is not None and self.services_tree.get_children():
            self.sort_treeview_column(self.current_sort_column, self.current_sort_reverse)

        self.update_buttons_state()

    def on_change_start_type_button_click(self):
        selected_service = self.services_tree.selection()
        if selected_service:
            service_name = self.services_tree.item(selected_service, 'text')
            current_start_type = self.wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[1]
            self.show_start_type_selection_dialog(service_name, current_start_type)

    def show_start_type_selection_dialog(self, service_name, current_start_type):
        self.SERVICE_AUTO_START = 2
        self.SERVICE_BOOT_START = 0
        self.SERVICE_DEMAND_START = 3
        self.SERVICE_DISABLED = 4
        self.SERVICE_SYSTEM_START = 1

        self.start_type_mapping = {
            "Boot": self.SERVICE_BOOT_START,
            "System": self.SERVICE_SYSTEM_START,
            "Auto": self.SERVICE_AUTO_START,
            "Manual": self.SERVICE_DEMAND_START,
            "Disabled": self.SERVICE_DISABLED,
            "Unknown": 0 
        }

        def confirm_and_change_start_type():
            new_start_type = selected_start_type.get()
            dialog.destroy()
            if new_start_type != self.start_type_mapping.get(current_start_type, 0):
                self.change_start_type(service_name, new_start_type)

        dialog = tk.Toplevel(self.root)
        dialog.title("Select Start Type")

        tk.Label(dialog, text="Choose new start type: ").pack()

        selected_start_type = tk.IntVar(value=self.start_type_mapping.get(current_start_type, 0))

        ttk.Radiobutton(dialog, text="Auto Start", variable=selected_start_type, value=self.SERVICE_AUTO_START).pack(anchor=tk.W)
        ttk.Radiobutton(dialog, text="Boot Start", variable=selected_start_type, value=self.SERVICE_BOOT_START).pack(anchor=tk.W)
        ttk.Radiobutton(dialog, text="Manual Start", variable=selected_start_type, value=self.SERVICE_DEMAND_START).pack(anchor=tk.W)
        ttk.Radiobutton(dialog, text="Disabled", variable=selected_start_type, value=self.SERVICE_DISABLED).pack(anchor=tk.W)
        ttk.Radiobutton(dialog, text="System Start", variable=selected_start_type, value=self.SERVICE_SYSTEM_START).pack(anchor=tk.W)

        confirm_button = ttk.Button(dialog, text="Confirm", command=confirm_and_change_start_type)
        confirm_button.pack()

        dialog.wait_window()

        return selected_start_type.get()

    def get_services(self):
        services_ptr = self.wsm_dll.EnumerateServicesWithInfo()
        services = []

        if self.wsm_dll.EnumerateServicesWithInfo():
            for i in range(self.wsm_dll.GetServicesCount()):
                services.append(services_ptr[i].decode("utf-8"))

        return services

    def wait_until_service_state(self, service_name, state):
        current_state = self.wsm_dll.GetServiceInfo(service_name.encode('utf-8')).decode('utf-8').split(",")[0]

        if current_state == state:
            self.update_services_tree()
            return
        else:
            if state == "Running":
                if current_state == "Stopped":
                    self.update_services_tree()
                    self.status_label.config(text=f"Service '{service_name}' has stopped unexpectedly.")
                    return
            elif state == "Stopped":
                if current_state == "Running":
                    self.update_services_tree()
                    self.status_label.config(text=f"Service '{service_name}' has started unexpectedly.")
                    return
            elif state == "Stopped(Restart)":
                if current_state == "Stopped":
                    self.update_services_tree()
                    self.start_service()
                    self.update_services_tree()
                    self.status_label.config(text=f"Service '{service_name}' has restarted successfully.")
                    return
            self.root.after(1000, self.wait_until_service_state, service_name, state)

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    app = WindowsServiceManager()
    app.run()