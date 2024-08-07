import sys

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QIntValidator
from PyQt6.QtWidgets import QMainWindow, QScrollArea, QLabel, QWidget, QComboBox, QVBoxLayout, QHBoxLayout, QPushButton, QGridLayout, QLineEdit

from widgets import TempModule, Relay, SpinBox, DTC

class Voltages(QMainWindow):
    def __init__(self, controller, model):
        super().__init__()
        self.setWindowTitle("Temperatures")
        self.controller = controller
        self.model = model
        self.is_changing_input = False
        self.start = 0
        self.end = 0
        self.volt = 0
        self.scroll = QScrollArea()
        self.widget = QWidget()
        
        # main layout
        layout = QVBoxLayout()

        # Inputs for cell voltages
        layout1 = QGridLayout()

        # Setting all voltages
        self.label = QLabel("Set All Voltages")
        self.all_volt_edit = QLineEdit()
        self.on_btn = QPushButton("Off")
        self.set_btn1 = QPushButton("Set")
        self.on_btn.setCheckable(True)

        layout1.addWidget(self.label, 0, 0)
        layout1.addWidget(self.all_volt_edit, 0, 1)
        layout1.addWidget(self.set_btn1, 0, 2)
        layout1.addWidget(self.on_btn, 0, 3)

        self.set_btn1.clicked.connect(self.set_all)

        self.set_range_label = QLabel("Set Voltage based on Range")
        layout1.addWidget(self.set_range_label, 1, 0)
        self.label1 = QLabel("Start Range")
        self.start_edit = QLineEdit(alignment=Qt.AlignmentFlag.AlignLeft)
        self.start_edit.setMaxLength(2)
        self.start_edit.setValidator(QIntValidator())
        layout1.addWidget(self.label1, 2, 0)
        layout1.addWidget(self.start_edit, 2, 1)

        self.label2 = QLabel("End Range")
        self.end_edit = QLineEdit(alignment=Qt.AlignmentFlag.AlignLeft)
        self.end_edit.setMaxLength(2)
        self.end_edit.setValidator(QIntValidator())
        layout1.addWidget(self.label2, 3, 0)
        layout1.addWidget(self.end_edit, 3, 1)

        self.label3 = QLabel("Set Voltage")
        self.volt_edit = QLineEdit(alignment=Qt.AlignmentFlag.AlignLeft)
        self.volt_edit.setMaxLength(3)
        layout1.addWidget(self.label3, 4, 0)
        layout1.addWidget(self.volt_edit, 4, 1)

        self.set_btn = QPushButton("Set Range")
        self.set_btn.clicked.connect(self.set_button)
        layout1.addWidget(self.set_btn, 5, 0)

        self.error_range_lbl = QLabel(" ")
        layout1.addWidget(self.error_range_lbl, 6, 0)

        # Module option
        self.input_label = QLabel("Choose Input")
        self.pot_btn = QPushButton("Potentiometer")
        self.pot_btn.setCheckable(True)
        self.laptop_btn = QPushButton("Laptop")
        self.laptop_btn.setCheckable(True)
        self.laptop_btn.setChecked(True)
        layout1.addWidget(self.input_label, 7, 0)
        layout1.addWidget(self.pot_btn, 7, 1)
        layout1.addWidget(self.laptop_btn, 7, 2)
        self.pot_btn.clicked.connect(self.change_input)
        self.laptop_btn.clicked.connect(self.stop_read)

        self.label4 = QLabel("Choose Module:")
        self.module = QComboBox()
        self.module.addItems([str(i) for i in range(1, 26)])
        layout1.addWidget(self.label4, 8, 0)
        layout1.addWidget(self.module, 8, 1)

        self.label5 = QLabel("Module Voltage:")
        print("module dict: ", self.model.module['1'])
        module = self.model.module['1']
        print("module dict: ", module)
        self.module_volt = QLineEdit(str(module))
        self.module_volt.setReadOnly(True)
        self.module.currentIndexChanged.connect(self.module_voltage)
        layout1.addWidget(self.label5, 9, 0)
        layout1.addWidget(self.module_volt, 9, 1)

        layout2 = QGridLayout()
        self.cell_voltages_label = QLabel("Cell Voltages")
        layout2.addWidget(self.cell_voltages_label, 0, 0)

        self.high_lbl = QLabel("Highest Cell Voltage")
        max_cell = self.model.calc_volt['Max_Cell']
        self.high_volt = QLineEdit(str(max_cell[1]))
        self.high_volt.setReadOnly(True)
        self.high_loc_lbl = QLabel("Cell Location")
        self.high_loc = QLineEdit(str(max_cell[0]))
        self.high_volt.setReadOnly(True)

        min_cell = self.model.calc_volt['Min_Cell']
        self.low_lbl = QLabel("Lowest Cell Voltage")
        self.low_volt = QLineEdit(str(min_cell[1]))
        self.low_volt.setReadOnly(True)
        self.low_loc_lbl = QLabel("Cell Location")
        self.low_loc = QLineEdit(str(min_cell[0]))
        self.low_volt.setReadOnly(True)

        self.avg_lbl = QLabel("Average Cell Voltage")
        self.avg_volt = QLineEdit(str(sum(self.model.voltages) / len(self.model.voltages)))
        self.avg_volt.setReadOnly(True)

        layout2.addWidget(self.high_lbl, 1, 0)
        layout2.addWidget(self.high_volt, 1, 1)
        layout2.addWidget(self.low_lbl, 2, 0)
        layout2.addWidget(self.low_volt, 2, 1)
        layout2.addWidget(self.avg_lbl, 3, 0)
        layout2.addWidget(self.avg_volt, 3, 1)

        layout.addLayout(layout1)
        layout.addLayout(layout2)

        layout3 = QVBoxLayout()
        for i in range(200):
            self.spin_box = SpinBox(i, model.voltages[i], self.controller)
            layout3.addWidget(self.spin_box)

        layout.addLayout(layout3)
        self.widget.setLayout(layout)

        self.scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOn)
        self.scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.scroll.setWidgetResizable(True)
        self.scroll.setWidget(self.widget)

        self.setCentralWidget(self.scroll)

        self.controller.model.updateVoltages.connect(self.update_voltages)
        self.controller.model.updatePot.connect(self.update_pot)
    
    def update_voltages(self, update):
        if update:
            curr_max = float(self.high_volt.text())
            high = self.model.calc_volt['Max_Cell']
            if curr_max != high[1]:
                self.high_volt.setText(str(high[1]))
                self.high_loc.setText(str(high[0] + 1))
                data_high = "MaxCell:" + str(high[1])
                self.controller.send_data(data_high)

            curr_min = float(self.low_volt.text())
            low = self.model.calc_volt['Min_Cell']
            if curr_min != low[1]:
                self.low_volt.setText(str(low[1]))
                self.low_loc.setText(str(low[0] + 1))
                data_low = "MinCell:" + str(low[1])
                self.controller.send_data(data_low)

            curr_avg= float(self.avg_volt.text())
            if curr_avg != self.model.calc_volt['Average_Cell']:
                self.avg_volt.setText(str(self.model.calc_volt['Average_Cell']))
                data_avg = "AverageVolt:" + str(self.model.calc_volt['Average_Cell'])
                self.controller.send_data(data_avg)
    
    def set_button(self):
        self.start = int(self.start_edit.text())
        self.end = int(self.end_edit.text())
        if self.end <= self.start:
            self.error_range_lbl.setText("Error! End has to be greater than start.")
        elif self.start == self.end or (self.start == 0 and self.end == 0):
            self.error_range_lbl.setText("Error! Cell ranges have to be specified.")
        else:
            self.error_range_lbl.setText(" ")
            self.controller.handle_set_voltage_range(self.start, self.end, float(self.volt_edit.text()))
    
    def set_all(self):
        volt = float(self.all_volt_edit.text())
        if volt:
            self.on_btn.setChecked(True)
            self.controller.handle_set_voltage_range(0, 200, volt)
    
    def module_voltage(self):
        index = self.module.currentIndex() + 1
        voltage = self.model.module[str(index)]
        self.module_volt.setText(str(voltage))
    
    def change_input(self, checked):
        if self.is_changing_input:
            return
        self.is_changing_input = True
        print("change_input called, checked:", checked)

        if checked:
            self.laptop_btn.setChecked(False)
            for i, val in enumerate(self.model.pot):
                print(f"Setting voltage for cell {i} to {val}")
                self.controller.handle_change_voltage(i, val)
        
        self.is_changing_input = False
    
    def update_pot(self, id, val):
        if self.pot_btn.isChecked():
            self.controller.handle_set_voltage_range(id*8,id*8+8, val)
            self.update_voltages(True)

    def stop_read(self, checked):
        if checked:
            self.pot_btn.setChecked(False)
            self.controller.handle_set_voltage_range(0, 200, 0)


class Temperatures(QMainWindow):
    def __init__(self, controller, model):
        super().__init__()
        self.setWindowTitle("Temperatures")
        self.controller = controller
        self.model = model
        self.scroll = QScrollArea()
        self.widget = QWidget()
        self.temps = [0] * 50

        layout = QVBoxLayout()

        #coolant values
        layout1 = QGridLayout()
        self.label = QLabel("Coolant Temperatures")
        self.label1 =  QLabel("Coolant 1")
        self.temp1 =QLineEdit(str(self.model.coolant[0]))

        self.label2 =  QLabel("Coolant 2")
        self.temp2 =QLineEdit(str(self.model.coolant[1]))
        
        layout1.addWidget(self.label, 0, 0)
        layout1.addWidget(self.label1, 1, 0)
        layout1.addWidget(self.temp1, 1, 1)
        layout1.addWidget(self.label2, 2, 0)
        layout1.addWidget(self.temp2, 2, 1)

        self.module_label = QLabel("Module Temperatures")
        self.min_temp_lbl = QLabel("Min. Temperature")
        min_temp = min(self.model.temps)
        self.min_temp = QLineEdit(str(min_temp))
        self.min_temp.setReadOnly(True)
        self.temp_loc_lbl = QLabel("Temp Location")
        min_temp_loc = self.model.temps.index(min_temp)
        self.min_temp_loc = QLineEdit(str(min_temp_loc + 1))
        self.min_temp_loc.setReadOnly(True)
        self.max_temp_lbl = QLabel("Max. Temperature")
        max_temp = max(self.model.temps)
        self.max_temp = QLineEdit(str(max_temp))
        self.max_temp.setReadOnly(True)
        self.max_loc_lbl = QLabel("Temp Location")
        max_temp_loc = self.model.temps.index(max_temp)
        self.max_temp_loc = QLineEdit(str(max_temp_loc + 1))
        self.max_temp.setReadOnly(True)
        self.avg_temp_lbl = QLabel("Average Temperature")
        avg_temp = sum(self.model.temps)/len(self.model.temps)
        self.avg_temp = QLineEdit(str(avg_temp))
        self.avg_temp.setReadOnly(True)

        layout1.addWidget(self.module_label, 3, 0)
        layout1.addWidget(self.min_temp_lbl, 4, 0)
        layout1.addWidget(self.min_temp, 4, 1)
        layout1.addWidget(self.temp_loc_lbl, 5, 0)
        layout1.addWidget(self.min_temp_loc, 5, 1)
        layout1.addWidget(self.max_temp_lbl, 6, 0)
        layout1.addWidget(self.max_temp, 6, 1)
        layout1.addWidget(self.max_loc_lbl, 7, 0)
        layout1.addWidget(self.max_temp_loc, 7, 1)
        layout1.addWidget(self.avg_temp_lbl, 8, 0)
        layout1.addWidget(self.avg_temp, 8, 1)
        layout.addLayout(layout1)

        #temp values
        for i in range(50):
            self.module = TempModule(i, self.model.temps[i], self.controller)
            layout.addWidget(self.module)

        self.widget.setLayout(layout)

        self.scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOn)
        self.scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.scroll.setWidgetResizable(True)
        self.scroll.setWidget(self.widget)

        self.setCentralWidget(self.scroll)

         # Connect model signal to update slot
        self.controller.model.coolantChanged.connect(self.update_coolant)
        self.controller.model.updateTemps.connect(self.update_temp)
    
    def update_coolant(self, id):
        if id == 1:
            temp = self.model.coolant[0]
            self.temp1.setText(str(temp))
        elif id == 2:
            temp = self.model.coolant[1]
            self.temp2.setText(str(temp))
    
    def update_temp(self, update):
        if update:
            high = self.model.calc_temps['Max_Temp']
            self.max_temp.setText(str(high[1]))
            self.max_temp_loc.setText(str(high[0] + 1))
            data_high = "MaxTemp:" + str(high[1])
            self.controller.send_data(data_high)
            low = self.model.calc_temps['Min_Temp']
            self.min_temp.setText(str(low[1]))
            self.min_temp_loc.setText(str(low[0] + 1))
            data_low = "MinTemp:" +  str(low[1])
            self.controller.send_data(data_low)
            self.avg_temp.setText(str(self.model.calc_temps['Average_Temp']))
            data_avg = "AverageTemp:" + str(self.model.calc_temps['Average_Temp'])
            self.controller.send_data(data_avg)

class Relays(QWidget):
    def __init__(self, controller, model):
        super().__init__()
        self.setWindowTitle("Relay Controls")
        layout = QVBoxLayout()
        
        layout1 = QGridLayout()
        self.breaktor = Relay(1, controller, model)
        layout1.addWidget(self.breaktor, 0,0)
        self.relay1 = Relay(2, controller, model)
        layout1.addWidget(self.relay1, 1, 0)
        self.relay2 = Relay(3, controller,  model)
        layout1.addWidget(self.relay2, 2, 0)
        self.relay3 = Relay(4, controller, model)
        layout1.addWidget(self.relay3, 3, 0)
        self.relay4 = Relay(5, controller, model)
        layout1.addWidget(self.relay4, 4, 0)

        layout.addLayout(layout1)
        self.setLayout(layout)

class Routines(QWidget):
    def __init__(self,  controller, model):
        super().__init__()
        self.controller = controller
        self.model = model

        self.layout = QVBoxLayout()
        layout1 = QGridLayout()
        self.routine_label = QLabel("Input Routine")
        self.label = QLabel("If there is a space in the routine, use and underscore(_)")
        self.routine = QLineEdit()
        self.send = QPushButton("Send")
        self.label = QLabel("PWM")
        self.pwm_edit = QLineEdit("")
        self.duty_label = QLabel("Duty Cycle")
        self.duty_edit = QLineEdit("")
        self.freq_label = QLabel("Frequency")
        self.freq_edit = QLineEdit("")

        self.hv_cur_pri_lbl = QLabel("HV Current Primary")
        self.hv_cur_pri = QLineEdit()
        self.set_current_pri = QPushButton("Set")
        self.hv_cur_fa_lbl = QLabel("HV Current Primary FA")
        self.hv_cur_fa = QLineEdit()
        self.set_current_fa = QPushButton("Set")

        self.hv_cur_sec_lbl = QLabel("HV Current Secondary")
        self.hv_cur_sec = QLineEdit()
        self.set_current_sec = QPushButton("Set")
        
        layout1.addWidget(self.label, 0, 0)
        layout1.addWidget(self.pwm_edit, 0, 1)
        layout1.addWidget(self.duty_label, 1, 0)
        layout1.addWidget(self.duty_edit, 1, 1)
        layout1.addWidget(self.freq_label, 2, 0)
        layout1.addWidget(self.freq_edit, 2, 1)
        layout1.addWidget(self.hv_cur_pri_lbl, 3, 0)
        layout1.addWidget(self.hv_cur_pri, 3, 1)
        layout1.addWidget(self.set_current_pri, 3, 2)
        layout1.addWidget(self.hv_cur_sec_lbl, 4, 0)
        layout1.addWidget(self.hv_cur_sec, 4, 1)
        layout1.addWidget(self.set_current_sec, 4, 2)
        layout1.addWidget(self.hv_cur_fa_lbl, 5, 0)
        layout1.addWidget(self.hv_cur_fa, 5, 1)
        layout1.addWidget(self.set_current_fa, 5, 2)
        layout1.addWidget(self.routine_label, 6, 0)
        layout1.addWidget(self.routine, 6, 1)
        layout1.addWidget(self.send, 6, 2)
        
        self.set_current_pri.clicked.connect(self.send_current_prim)
        self.set_current_fa.clicked.connect(self.send_current_fa)
        self.set_current_sec.clicked.connect(self.send_current_sec)
        self.send.clicked.connect(self.send_routine)
        
        self.controller.model.pwmChanged.connect(self.update_pwm)

        #DTC part
        dtc_label = QLabel("DTCs")
        self.controller.model.updateDTC.connect(self.produce_dtc)
        
        self.layout.addLayout(layout1)
        self.layout.addWidget(dtc_label)

        self.dtc_layout = QVBoxLayout()
        self.layout.addLayout(self.dtc_layout)
        self.setLayout(self.layout)


    def send_routine(self):
        #data = "Routine:" + self.routine.text()
        data = self.routine.text()
        self.controller.send_data(data)
    
    def send_current_prim(self):
        data = "HV_current_Pri:" + self.hv_cur_pri.text()
        self.controller.send_data(data)
    
    def send_current_sec(self):
        data = "HV_current_Sec:" + self.hv_cur_sec.text()
        self.controller.send_data(data)

    def send_current_fa(self):
        data = "HV_current_Pri_FA:" + self.hv_cur_fa.text()
        self.controller.send_data(data)

    def update_pwm(self, state):
        if state:
            self.pwm_edit.setText("Connected")
            duty = self.model.pwm_desc['Duty_Cycle']
            self.duty_edit.setText(str(duty))
            freq = self.model.pwm_desc['Frequency']
            self.freq_edit.setText(str(freq))
        else:
            self.pwm_edit.setText("Disconnected")
    
    def produce_dtc(self, state):
        if state:
            # Remove all existing widgets in dtc_layout
            for i in reversed(range(self.dtc_layout.count())):
                widget = self.dtc_layout.itemAt(i).widget()
                if widget is not None:
                    widget.deleteLater()  # Properly delete the widget
            # Add new DTC widgets
            for code, desc in self.model.dtc.items():
                dtc_widget = DTC(code, desc)
                self.dtc_layout.addWidget(dtc_widget)
