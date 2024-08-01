import sys

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QIntValidator
from PyQt6.QtWidgets import QMainWindow, QScrollArea, QLabel, QWidget, QComboBox, QVBoxLayout, QHBoxLayout, QPushButton, QGridLayout, QLineEdit

from widgets import Slider, TempModule, Relay, SpinBox

class Voltages(QMainWindow):
    def __init__(self, controller, model):
        super().__init__()
        self.setWindowTitle("Temperatures")
        self.controller = controller
        self.model = model
        self.start = 0
        self.end = 0
        self.volt = 0
        self.scroll = QScrollArea()
        self.widget = QWidget()
        
        #main layout
        layout = QVBoxLayout()


        ##inputs for cell voltages
        layout1 = QGridLayout()

        # setting all voltages
        self.label = QLabel("Set All Voltages")
        self.volt_edit = QLineEdit()
        self.on_btn = QPushButton("Off")
        self.set_btn = QPushButton("Set")


        layout1.addWidget(self.label, 0, 0)
        layout1.addWidget(self.volt_edit, 0, 1)
        layout1.addWidget(self.set_btn, 0, 2)
        layout1.addWidget(self.on_btn, 0, 3)

        self.set_range_label = QLabel("Set Voltage based on Range")
        layout1.addWidget(self.set_range_label, 1, 0)
        self.label1 = QLabel("Start Range")
        self.start_edit = QLineEdit(alignment=Qt.AlignmentFlag.AlignLeft)
        self.start_edit.setMaxLength(2)
        self.start_edit.setValidator(QIntValidator())
        layout1.addWidget(self.label1,2,0 )
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

        self.error_range_lbl= QLabel(" ")
        layout1.addWidget(self.error_range_lbl, 6, 0)

        # module option
        self.input_label= QLabel("Choose Input")
        self.pot_btn =  QPushButton("Potentiometer")
        self.pot_btn.setCheckable(True)
        self.laptop_btn = QPushButton("Laptop")
        self.laptop_btn.setCheckable(True)
        layout1.addWidget(self.input_label, 7, 0)
        layout1.addWidget(self.pot_btn, 7, 1)
        layout1.addWidget(self.laptop_btn, 7, 2)
        self.pot_btn.clicked.connect(self.change_input)

        self.label4 = QLabel("Choose Module:")
        self.module = QComboBox()
        self.module.addItems(['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25'])
        layout1.addWidget(self.label4, 8, 0)
        layout1.addWidget(self.module, 8, 1)

        self.label5 = QLabel("Module Voltage:")
        self.module_volt = QLineEdit(self.model.module['1'])
        self.module_volt.setReadOnly(True)
        self.module.currentIndexChanged.connect(self.module_voltage)
        layout1.addWidget(self.label5, 9, 0)
        layout1.addWidget(self.module_volt, 9, 1)

       #self.high_mod_lbl = QLabel("Highest Module Voltage")
        

        layout2 = QGridLayout()
        self.cell_voltages_label = QLabel("Cell Voltages")
        layout2.addWidget(self.cell_voltages_label, 0,  0)

        self.high_lbl = QLabel("Highest Cell Voltage")
        self.high_volt = QLineEdit(str(max(self.model.voltages)))
        self.high_volt.setReadOnly(True)
        self.low_lbl = QLabel("Lowest Cell Voltage")
        self.low_volt = QLineEdit(str(min(self.model.voltages)))
        self.low_volt.setReadOnly(True)
        self.avg_lbl = QLabel("Average Cell Voltage")
        self.avg_volt =QLineEdit(str(sum(self.model.voltages) / len(self.model.voltages)))
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
    
    def set_button(self):
        self.start = int(self.start_edit.text())
        self.end = int(self.end_edit.text())
        if self.end <= self.start:
            self.error_range_lbl.setText("Error! End has to be greater than start.")
        elif self.start == self.end or (self.start == 0 and self.end == 0):
            self.error_range_lbl.setText("Error! Cell ranges has to be specified.")
        else:
            self.error_range_lbl.setText(" ")
            self.controller.handle_set_voltage_range(self.start, self.end, float(self.volt_edit.text()))
    
    def module_voltage(self):
        index = self.module.currentIndex() + 1
        voltage = self.model.module[str(index)]
        self.module_volt.setText(str(voltage))
    
    def change_input(self, checked):
        while checked:
            self.laptop_btn.setChecked(False)
            self.controller.read_data()
    
    def stop_read(self, checked):
        if checked:
            self.pot_btn.setChecked(False)

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
    
    def update_coolant(self, id, temp):
        if id == 0:
            self.temp1.setText(str(temp))
        else:
            self.temp2.setText(str(temp))

class Relays(QWidget):
    def __init__(self, controller, model):
        super().__init__()
        self.setWindowTitle("Relay Controls")
        layout = QVBoxLayout()
        
        self.relay1 = Relay(1, controller, model)
        layout.addWidget(self.relay1)
        self.relay2 = Relay(2, controller, model)
        layout.addWidget(self.relay2)
        self.relay3 = Relay(3, controller,  model)
        layout.addWidget(self.relay3)
        self.relay4 = Relay(4, controller, model)
        layout.addWidget(self.relay4)
        self.breaktor = Relay(5, controller, model)
        layout.addWidget(self.breaktor)

        self.setLayout(layout)

class Routines(QWidget):
    def __init__(self,  controller, model):
        super().__init__()
        self.controller = controller
        self.model = model

        layout = QGridLayout()
        self.routine_label = QLabel("Input Routine")
        self.routine = QLineEdit()
        self.send = QPushButton("Send")
        self.label = QLabel("PWM")
        self.pwm_edit = QLineEdit("")

        layout.addWidget(self.routine_label, 0, 0)
        layout.addWidget(self.routine, 0, 1)
        layout.addWidget(self.send, 0, 2)
        layout.addWidget(self.label, 1, 0)
        layout.addWidget(self.volt_edit, 1, 1)

        self.send.clicked.connect(self.send_routine)
        
        self.setLayout(layout)
    
    def send_routine(self):
        data = "Routine " + self.routine.text()
        self.controller.send_data(data)
        
