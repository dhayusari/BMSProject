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
    coolantChanged = pyqtSignal(int)
    updateVoltages = pyqtSignal(bool)
    updateTemps = pyqtSignal(bool)
    pwmChanged = pyqtSignal(bool)
    updateDTC = pyqtSignal(str, int)
    updatePot = pyqtSignal(int, float)

    def __init__(self):
        super().__init__()
        self.voltages = [0.0] * 200
        self.pot = [0] * 200
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
        self.dtc = {}
    
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

        min_volt = 6
        max_volt = 0
        #change min & max of voltage
        for module in self.module.keys():
            volt = self.module[module]
            if volt <= min_volt:
                self.calc_volt['Lowest_Module_Volt'] = [int(module), volt]
            elif volt >= max_volt:
                self.calc_volt['Highest_Module_Volt'] = [int(module, volt)]
    
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
        self.coolantChanged.emit(id)

    def change_pwm(self, state):
        self.pwm = state
        self.pwmChanged.emit(state == 1)
    
    def change_pot(self, num, volt):
        for i in range(num*8, num*8 + 8, 1):
            self.pot[i] = volt
        self.updatePot.emit(num, volt)

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

    def update_dtc(self, code, condition):
        if condition:
            desc = ""
            if code == "P0DE7":
                desc = "Max Cell Voltage >= 4.25 V"
            elif code == "P1C01":
                desc = "Max Cell Voltage >= 4.35 V"
            elif code == "P0DE6":
                desc = "Min Cell Voltage <= 2.8 V"
            elif code == "P1C00":
                desc = "Min Cell Voltage <= 1.7 V"
            elif code == "P1A9B":
                desc = "Max Module Temp >= 58 C"
            elif code == "P0A7E":
                desc = "Max Module Temp >= 65 C"
            elif code == "P1A9A":
                desc = "MinTModule Temp <= -37"
            elif code == "P0CA7":
                desc = "HV Current Primary >= 1350 A, HV Current Primary FA is 1 and HV Current Secondary >= 1350 A, or HV Current Primary <= -1350 A"
            elif code == "P0A0A":
                desc = "HVIL circuit open"
            elif code == "P0A0B":
                desc = "HVIL Performance Low"
            elif code == "P0A0C":
                desc = "HVIL Circuit Low"
            elif code == "P29FF":
                desc = "Thermal Runway: Min Cell is < 2.1 V and difference between Temps are either 2 C or 15 C"
            self.dtc[str(code)] = desc
        else:
            del self.dtc[code]
        
        self.updateDTC.emit(1)

class Controller:
    def __init__(self, model):
        self.model = model
        self.serial_port = serial.Serial('com11', 115200, timeout=1)
        self.worker = Worker(self.serial_port)
        self.worker.data_received.connect(self.read_data)
        self.worker.start()

    def __del__(self):
        self.worker.stop()
        print("Worker has stopped")
        self.serial_port.close()

    def send_data(self, data):
        print("Data being sent: ", data)
        self.serial_port.write((data + '\n').encode('utf-8'))
    
    def read_data(self, data):
        print("Reading data: ", data)
        pattern1 = r"pot([\d.]+):\s*([\d.]+)"
        pattern2 = r"Temp([\d.]+):\s*([\d.]+)"
        pattern3 = r"DTC\s*([\w\d]+)\s(Demature|Mature)"
        pattern4 = r"PWM: ([\d])"
        pot_match = re.findall(pattern1, data)
        temp_match = re.findall(pattern2, data)
        dtc_match = re.findall(pattern3, data)
        pwm_match = re.search(pattern4, data)

        if pot_match:
            for match in pot_match:
            # print("\npattern1 matched. \n")
                cell_num = int(match[0])
                cell_value = float(match[1])
                self.handle_set_pot(cell_num, cell_value)
        if temp_match:
            for match in temp_match:
            #print("\npattern2 matched. \n")
                temp_num = int(match[0])
                temp_value = float(match[1])
                self.handle_change_coolant(temp_num, temp_value)
        if dtc_match:
            for match in dtc_match:
                code = match.group(1)
                if match.group(2) == "Mature":
                    self.model.update_dtc(code, 1)
                else:
                    self.model.update_dtc(code, 0)
        if pwm_match:
            self.model.update_pwm(int(pwm_match.group(1)))
    
    def handle_set_voltage_range(self, start, end, volt):
        self.model.set_range_voltage(int(start), int(end), volt)
        # for i in range(int(start) - 1, int(end), 1):
        #     self.handle_change_voltage(i, volt)
    
    def handle_set_pot(self, module, volt):
        self.model.change_pot(module, volt)
    
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
