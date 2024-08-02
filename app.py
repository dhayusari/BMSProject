# app.py
import sys
from PyQt6.QtGui import *
from PyQt6.QtCore import *
from PyQt6.QtWidgets import *
import serial
import re
from pages import Voltages, Temperatures, Relays, Routines

class Worker(QThread):
    data_received = pyqtSignal(str)

    def __init__(self, serial_port):
        super().__init__()
        self.serial_port = serial_port
        self._running = True

    def run(self):
        while self._running:
            if self.serial_port.in_waiting > 0:
                data = self.serial_port.readline().decode('utf-8', errors='replace').strip()
                self.data_received.emit(data)


    def stop(self):
        self._running = False
        self.wait()

class Data(QObject):
    voltageChanged = pyqtSignal(int, float)
    tempChanged = pyqtSignal(int, int)
    relayToggled = pyqtSignal(int, bool)
    coolantChanged = pyqtSignal(int, int)
    updateVoltages = pyqtSignal(bool)
    updateTemps = pyqtSignal(bool)
    pwmChanged = pyqtSignal(bool)

    def __init__(self):
        super().__init__()
        self.voltages = [0.0] * 200
        self.module = {}
        self.calculate_module()
        self.calc_volt = {
            'Min_Cell': [0, 0],
            'Max_Cell': [0,0],
            'Average_Cell': 0,
            'Highest_Module_Volt': [0,0],
            'Lowest_Module_Volt': [0,0]
        }
        self.temps = [0] * 50
        self.calc_temps = {
            'Min_Temp': 0,
            'Max_Temp': 0,
            'Average_Temp': 0
        }
        self.relays = [0] * 5 #initially open
        self.coolant = [0] * 2
        self.pwm = 0
    
    def calculate_module(self):
        for i in range(25):
            module = i * 8
            average = sum(self.voltages[module:module+8]) / 8
            self.module[str(module // 8 + 1)] = average
        print(self.module)
    
    def update_module(self, cell_num):
        module = cell_num // 8
        print("Module ", str(module + 1), " Updated!")
        average = sum(self.voltages[cell_num:cell_num + 8]) / 8
        self.module[str(module + 1)] = average
    
    def change_voltage(self, cell_num, volt):
        print("Changed Cell Voltage:", cell_num + 1)
        self.voltages[cell_num] = volt
        self.voltageChanged.emit(cell_num, volt)
        self.update_module(cell_num)
        self.update_calc_volt()
    
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
        self.update_calc_temp()
    
    def toggle_relay(self, id):
        print("Relay toggled!")
        self.relays[id - 1] = not self.relays[id - 1]
        self.relayToggled.emit(id, self.relays[id - 1])

    def change_coolant(self, id, temp):
        print("Coolant:  ", id)
        print("Temp: ", temp)
        self.coolant[id - 1]= temp
        self.coolantChanged.emit(id, temp)

    def change_pwm(self, state):
        self.pwm = state
        self.pwmChanged.emit(state == 1)

    def update_calc_volt(self):
        #finding min
        min_val = min(self.voltages)
        min_id = self.voltages.index(min_val)
        self.calc_volt['Min_Cell'] = [min_id, min_val]
        #finding max
        max_val = max(self.voltages)
        max_id = self.voltages.index(max_val)
        self.calc_volt['Max_Cell'] = [max_id, max_val]
        #finding average
        avg_volt = sum(self.voltages) / len(self.voltages)
        self.calc_volt['Average_Cell'] = avg_volt
        self.updateVoltages.emit(True)
    
    def update_calc_temp(self):
        #finding min
        min_val = min(self.temps)
        min_id = self.temps.index(min_val)
        self.calc_temps['Min_Temp'] = [min_id, min_val]
        #finding max
        max_val = max(self.temps)
        max_id = self.temps.index(max_val)
        self.calc_temps['Max_Temp'] = [max_id, max_val]
        #finding average
        avg_temp = sum(self.temps) / len(self.voltages)
        self.calc_temps['Average_Temp'] = avg_temp
        self.updateTemps.emit(True)


class Controller:
    def __init__(self, model):
        self.model = model
        # self.serial_port = serial.Serial('com11', 115200, timeout=1)
        # self.worker = Worker(self.serial_port)
        # self.worker.data_received.connect(self.read_data)
        # self.worker.start()

    def __del__(self):
        # self.worker.stop()
        print("Worker has stopped")
        # self.serial_port.close()

    def send_data(self, data):
        print("Data being sent: ", data)
        # self.serial_port.write((data + '\n').encode('utf-8'))
    
    def read_data(self, data):
        print("Reading data: ", data)
        pattern1 = r"pot([\d.]+):\s*([\d.]+)"
        pattern2 = r"Temp([\d.]+):\s*([\d.]+)"
        cell_match = re.search(pattern1, data)
        temp_match = re.search(pattern2, data)

        if cell_match:
            # print("\npattern1 matched. \n")
            cell_num = int(cell_match.group(1))
            cell_value = float(cell_match.group(2))
            self.handle_set_voltage_range((cell_num - 1) * 8, (cell_num - 1) * 8 + 8, cell_value)
        if temp_match:
            #print("\npattern2 matched. \n")
            temp_num = int(temp_match.group(1))
            temp_value = float(temp_match.group(2))
            self.handle_change_coolant(temp_num, temp_value)
        else:
            pass
            #print("\nDID not match. data read is: \n", data)
    
    def handle_set_voltage_range(self, start, end, volt):
        self.model.set_range_voltage(int(start), int(end), volt)
        for i in range(int(start) - 1, int(end), 1):
            self.handle_change_voltage(i, volt)
    
    def handle_change_voltage(self, cell_num, volt):
        self.model.change_voltage(cell_num, volt)
        cell = "Cell" + str(cell_num + 1) + ":" + str(volt)
        self.send_data(cell)
    
    def handle_change_temp(self, temp_num, temp):
        self.model.change_temp(temp_num, temp)
        cell_temp = "Temp" + str(temp_num + 1) + ":" + str(temp)
        self.send_data(cell_temp)

    def handle_relay_toggle(self, id, state):
        if state:
            on_off = 1
        else:
            on_off = 0
        self.model.toggle_relay(id)
        relay = "Relay" + str(id) + ":" + str(on_off)
        self.send_data(relay)
    
    def handle_change_coolant(self, id, value):
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
        self.counter += 1


class Tabs(QWidget):
    def __init__(self, parent, controller, model):
        super(QWidget, self).__init__(parent)
        self.controller = controller
        self.model = model
        self.layout = QVBoxLayout(self)
        
        self.tabs = QTabWidget()
        self.tab1 = Voltages(self.controller, self.model)
        self.tab2 = Temperatures(self.controller, self.model)
        self.tab3 = Relays(self.controller, self.model)
        self.tab4 = Routines(self.controller, self.model)
        self.tabs.resize(300,200)
        
        self.tabs.addTab(self.tab1, "Voltages")
        self.tabs.addTab(self.tab2, "Temperatures")
        self.tabs.addTab(self.tab3, "Relays")
        self.tabs.addTab(self.tab4, "DTC and Routines")
        
        self.layout.addWidget(self.tabs)
        self.setLayout(self.layout)


if __name__ == "__main__":
    app = QApplication([])
    model = Data()
    controller = Controller(model)
    window = MainWindow(controller, model)
    window.show()
    app.exec()
