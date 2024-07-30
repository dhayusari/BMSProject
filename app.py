import sys
from PyQt6.QtGui import *
from PyQt6.QtCore import *
from PyQt6.QtWidgets import *
import serial
import re

from pages import Voltages, Temperatures, Relays, Routines
# class Worker(QRunnable):
#     def __init__(self, fn, *args, **kwargs):
#         super(Worker, self).__init__()
#         self.fn = fn
#         self.args = args
#         self.kwargs = kwargs

#     def run(self):
#         self.fn(*self.args, **self.kwargs)


class Data(QObject):
    voltageChanged = pyqtSignal(int, float)
    tempChanged = pyqtSignal(int, int)
    relayToggled = pyqtSignal(int, bool)

    def __init__(self):
        super().__init__()
        self.voltages = [0.0] * 200
        self.temps = [0] * 50
        self.relays = [0] * 5
        self.coolant = [0] * 2
    
    def change_voltage(self, cell_num, volt):
        print("Changed Cell Voltage:", cell_num + 1)
        self.voltages[cell_num] = volt
        self.voltageChanged.emit(cell_num, volt)
    
    def set_range_voltage(self, start, end, volt):
        print("Change Voltage Cell Range")
        print("Start Range: ", start)
        print("End Range: ", end)
        for i in range(start - 1, end - 1, 1):
            print("Changed Cell Voltage:", i + 1)
            self.voltages[i] = volt
            self.voltageChanged.emit(i, volt)
    
    def change_temp(self, temp_num, temp):
        print("Changed Temp Number: ", temp_num + 1)
        self.temps[temp_num] = temp
        self.tempChanged.emit(temp_num, temp)
    
    def toggle_relay(self, id):
        print("Relay toggled!")
        self.relays[id - 1] = not self.relays[id - 1]
        self.relayToggled.emit(id, self.relays[id - 1])

    def change_coolant(self, id, temp):
        print("Coolant:  ", id)
        print("Temp: ", temp)
        self.coolant[id - 1]= temp


class Controller:
    # Class that handles clicks and changes in UI 

    def __init__(self, model):
        self.model = model
        #self.serial_port = serial.Serial('com4', 115200, timeout = 1)

        # try:
        #     while True:
        #         if self.serial_port.in_waiting > 0:
        #             data = self.serial_port.readline().decode('utf-8').strip()
        #             print(f"Received: {data}")
        #             self.read_data(data)
        # finally:
        #     self.serial_port.close()

    def send_data(self, data):  
        print("Data being sent: ", data)
        #self.serial_port.write((data + '\n').encode('utf-8'))
    
    def read_data(self):
        #if self.serial_port.in_waiting > 0:
        #    data = self.serial_port.readline().decode('utf-8').strip()           
        print("Reading data")
    
        # pattern1 = r"pot\s([\d.]+):\s*([\d.]+)"

        # pattern2 = r"Coolant\s([\d.]+):\s*([\d.]+)"

        # #pattern3 = r"Pot\s([\d.]+):\s*([\d.]+)"

        # cell_match = re.search(pattern1, data) 

        # temp_match = re.search(pattern2, data)

        # #pots_match = re.search(pattern3, data) 

        # if cell_match: 
        #     cell_num = int(cell_match.group(1))
        #     cell_value = float(cell_match.group(2))
        #     self.handle_set_voltage_range((cell_num - 1) * 8, (cell_num - 1) * 8 + 8, cell_value)
        # if temp_match:
        #     temp_num = int(temp_match.group(1))
        #     temp_value = float(temp_match.group(2))
        #     self.handle_change_coolant(temp_num, temp_value)
        # # if pots_match:
        #     pots_values = float(pots_match.group(1))
        #     for i, pot_value in enumerate(pots_values, start = 1):
        #         self.change_pots(i, int(pot_value))
        # ser = serial.Serial(port = 'COM3')
        

    def handle_set_voltage_range(self, start, end, volt):
        #update values in model
        self.model.set_range_voltage(int(start), int(end), volt)
        #sending data
        for i in range(int(start) - 1, int(end), 1):
            self.handle_change_voltage(i, volt)
    
    def handle_change_voltage(self, cell_num, volt):
        #update value in model
        self.model.change_voltage(cell_num, volt)
        cell = "Cell " + str(cell_num + 1) + ": " + str(volt)
        self.send_data(cell)
    
    def handle_change_temp(self, temp_num, temp):
        #update value
        self.model.change_temp(temp_num, temp)
        cell_temp = "Temp " + str(temp_num + 1) + ": " + str(temp)
        self.send_data(cell_temp)

    def handle_relay_toggle(self, id, state):
        if state:
            on_off = 1
        else:
            on_off = 0
        self.model.toggle_relay(id)
        relay = "Relay " + str(id) + ": " + str(on_off)
        self.send_data(relay)
    
    def handle_change_coolant(self, id, value):
        #update value
        self.model.change_coolant(id, value)

class MainWindow(QMainWindow):

    def __init__(self, controller, model):
        super().__init__()
        self.counter = 0
        self.controller = controller
        self.model = model

        self.setWindowTitle("Miniature BMS")
        self.resize(500, 750)

        self.tabs = Tabs(self, self.controller, self.model)
        self.setCentralWidget(self.tabs)

        self.timer = QTimer()
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.recurring_timer)
        self.timer.start()
    
    def recurring_timer(self):
        # print("Counter: ", self.counter)
        self.counter += 1
        

class Tabs(QWidget):
    def __init__(self, parent, controller, model):
        super(QWidget, self).__init__(parent)
        self.controller = controller
        self.model = model
        self.layout = QVBoxLayout(self)
        
        # Initialize tab screen
        self.tabs = QTabWidget()
        self.tab1 = Voltages(self.controller, self.model)
        self.tab2 = Temperatures(self.controller, self.model)
        self.tab3 = Relays(self.controller, self.model)
        self.tab4 = Routines(self.controller, self.model)
        self.tabs.resize(300,200)
        
        # Add tabs
        self.tabs.addTab(self.tab1, "Voltages")
        self.tabs.addTab(self.tab2, "Temperatures")
        self.tabs.addTab(self.tab3, "Relays")
        self.tabs.addTab(self.tab4, "Routines")
        
        # Add tabs to widget
        self.layout.addWidget(self.tabs)
        self.setLayout(self.layout)
        

if __name__ == "__main__":
    app = QApplication([])
    model = Data()
    controller = Controller(model)
    window = MainWindow(controller, model)
    window.show()
    app.exec()