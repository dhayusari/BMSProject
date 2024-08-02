from PyQt6.QtCore import *
from PyQt6.QtGui import *
from PyQt6.QtWidgets import QWidget, QSlider, QVBoxLayout, QHBoxLayout, QLabel, QGridLayout, QLineEdit, QPushButton, QDoubleSpinBox

class Slider(QWidget):
    def __init__(self, num, voltage, controller, parent=None):
        super(Slider, self).__init__(parent)
        self.setMinimumSize(100, 200)  
        self.controller = controller
        self.cell_idx = num
        name = "Cell " + str(num + 1)
        self.label = QLabel(name, self)
        self.slider = QSlider(self)
        self.slider.setOrientation(Qt.Orientation.Vertical)
        self.slider.setTickPosition(QSlider.TickPosition.TicksRight)
        self.slider.setMaximum(60)
        self.slider.setMinimum(0)
        self.slider.setTickInterval(1)
        self.slider.setValue(int(voltage))
        self.label2 = QLabel(str(voltage / 10), self)

        # Behaviour handling
        self.slider.valueChanged.connect(self.changed_value)
        self.slider.sliderReleased.connect(self.slider_released)

        # Layout
        self.layout = QVBoxLayout(self)
        self.layout.addWidget(self.label, alignment = Qt.AlignmentFlag.AlignCenter)
        self.layout.addWidget(self.slider, alignment = Qt.AlignmentFlag.AlignCenter)
        self.layout.addWidget(self.label2, alignment = Qt.AlignmentFlag.AlignCenter)
        self.layout.addStretch()

        # Connect model signal to update slot
        self.controller.model.voltageChanged.connect(self.update_slider)

    def changed_value(self, val):
        scaled = round(val / 10, 2)
        print((val / 10))
        self.label2.setText(f"{scaled}")  

    def slider_released(self):
        voltage = round(self.slider.value() / 10, 2)
        self.controller.handle_change_voltage(self.cell_idx, float(voltage))
        print("Slider released! Final Voltage:", f"{voltage}")

    def update_slider(self, cell_num, voltage):
        if cell_num == self.cell_idx:
            self.slider.setValue(int(voltage * 10))
            self.label2.setText(f"{voltage:.2f}")


class SpinBox(QWidget):
    def __init__(self, num, voltage, controller):
        super().__init__()
        self.controller = controller
        layout = QGridLayout()
        self.num = num

        #spin box thingy
        self.spin_box = QDoubleSpinBox()
        self.spin_box.setRange(0.0, 6.0)
        self.spin_box.setSingleStep(0.1)
        self.spin_box.setValue(voltage)

        cell = "Cell " + str(num +  1)
        self.label = QLabel(cell)
        self.set_button = QPushButton("Set")
        self.en_button = QPushButton("Off")
        self.en_button.setCheckable(True)

        layout.addWidget(self.label, 0, 0)
        layout.addWidget(self.spin_box, 0, 1)
        layout.addWidget(self.set_button, 0, 2)
        layout.addWidget(self.en_button, 0, 3)

        self.set_button.clicked.connect(self.set_voltage)
        self.en_button.clicked.connect(self.btn_enabled)
        self.setLayout(layout)

        # Connect model signal to update slot
        self.controller.model.voltageChanged.connect(self.update_voltage)

    def btn_enabled(self, checked):
        # enables volt to 3 V
        if checked:
            self.controller.handle_change_voltage(self.num, 3.00)
            self.spin_box.setValue(3.00)
            self.en_button.setText("On")
        else:
            self.controller.handle_change_voltage(self.num, 0)
            self.spin_box.setValue(0)
            self.en_button.setText("Off")

    def set_voltage(self):
        voltage = self.spin_box.value()
        self.en_button.setChecked(voltage > 0)
        self.en_button.setText("On")
        self.controller.handle_change_voltage(self.num, voltage)
    
    def update_voltage(self, cell_num, voltage):
        if cell_num == self.num:
            self.spin_box.setValue(voltage)
            


class TempModule(QWidget):
    def __init__(self, num, temp, controller):
        super().__init__()
        self.controller = controller
        self.temp_num = num
        layout = QGridLayout()

        if num % 2 == 0:
            module_name = "Module " + str(int(num / 2 + 1))
            self.module_label = QLabel(module_name)
            layout.addWidget(self.module_label, 0 , 0)
        else:
            self.empty_lbl = QLabel("      ")
            layout.addWidget(self.empty_lbl, 0, 0)

        temp_name = "Temp " + str(num + 1) + ":"
        self.temp_label = QLabel(temp_name)
        #self.temp2_label = QLabel(temp2_name)

        self.temp = QDoubleSpinBox()
        self.temp.setRange(0, 150)
        self.temp.setSingleStep(1.0)

        self.temp.setValue(temp)
        # self.temp2 = QLineEdit()
        # self.temp2.setText(str(self.model.temps[module * 2 + 1]))

        self.temp_btn = QPushButton(text="Off")
        self.temp_btn.setCheckable(True)

        self.set_btn = QPushButton(text="Set")
        
        # self.temp2_btn = QPushButton(text="Off")
        # self.temp2_btn.setCheckable(True)
        # self.temp2_btn.clicked.connect(self.button2_enabled)

        #layout.addWidget(self.module_label, 0, 0)
        layout.addWidget(self.temp_label, 0, 1)
        layout.addWidget(self.temp, 0, 2)
        #layout.addWidget(self.temp2_label, 1, 1)
        #layout.addWidget(self.temp2, 1, 2)
        layout.addWidget(self.set_btn, 0, 3)
        layout.addWidget(self.temp_btn, 0, 4)
        #layout.addWidget(self.temp2_btn, 1, 3)

        # Behaviour handling
        #self.temp.textChanged.connect(self.temp_changed)
        #self.temp2.textChanged.connect(self.temp2_changed)
        self.set_btn.clicked.connect(self.temp_changed)
        self.temp_btn.clicked.connect(self.button_enabled)
        #self.temp2_btn.clicked.connect(self.button2_enabled)

        self.setLayout(layout)

        # Connect model signal to update slot
        self.controller.model.tempChanged.connect(self.update_temps)

    def button_enabled(self, checked):
        # enables temp to 25 C
        if checked:
            self.controller.handle_change_temp(self.temp_num, 25)
            self.temp.setValue(25)
            self.temp_btn.setText("On")
        else:
            self.controller.handle_change_temp(self.temp_num, 0)
            self.temp.setValue(0)
            self.temp_btn.setText("Off")

#     def button2_enabled(self, checked):
#         # enables temp to 25 C
#         if checked:
#             self.controller.handle_change_temp(self.module_num * 2 + 1, 25)
#             self.temp2.setText("25")
#             self.temp2_btn.setText("On")
#         else:
#             self.controller.handle_change_temp(self.module_num * 2 + 1, 0)
#             self.temp2.setText("0")
#             self.temp2_btn.setText("Off")
    
    def temp_changed(self):
        if self.temp.value() > 0:
            self.temp_btn.setText("On")
            new_temp = float(self.temp.text())
            self.controller.handle_change_temp(self.temp_num , int(new_temp))
        else:
            print("Temp has not been changed")
    
#     def temp2_changed(self):
#         self.temp2_btn.setText("On")
#         new_temp = int(self.temp2.text())
#         self.controller.handle_change_temp(self.module_num * 2 + 1 , new_temp)

    def update_temps(self, temp_num, temp):
        if temp_num == self.temp_num:
            self.temp.setValue(temp)
            self.temp_btn.setChecked(temp >0 )
            self.temp_btn.setText("On" if temp > 0 else "Off")
        # elif temp_num == self.module_num * 2 + 1:
        #     self.temp2.setText(str(temp))
        #     self.temp2_btn.setChecked(temp >0)
        #     self.temp2_btn.setText("On" if temp > 0 else "Off")
    


class Relay(QWidget):
    def __init__(self, id, controller, model):
        super().__init__()
        self.controller = controller
        self.model = model
        self.id = id
        layout = QGridLayout()

        if id == 1:
            self.name = QLabel("Breaktor")
            self.precharge = QPushButton("Precharge")
            self.precharge.setCheckable(True)
            layout.addWidget(self.precharge, 0, 1)
            self.precharge.clicked.connect(self.precharge_routine)
        elif id == 2:
            self.name = QLabel("Battery Pack (+)")
        elif id == 3:
            self.name = QLabel("Battery Pack (-)")
        elif id == 4:
            self.name = QLabel("Bus (+)")
        else:
            self.name = QLabel("Bus (-)")

        self.button = QPushButton(text= "Open")
        self.button.setCheckable(True)

        if self.model.relays[id - 1]:
            self.button.setText("Close")
        
        self.button.clicked.connect(self.btn_clicked)

        layout.addWidget(self.name, 0, 0)
        layout.addWidget(self.button, 0, 2)

        self.setLayout(layout)

        # Connect model signal to update slot
        self.controller.model.relayToggled.connect(self.update_relay)

    def btn_clicked(self, checked):
        self.controller.handle_relay_toggle(self.id, checked)
        self.button.setText("Close" if checked else "Open")

    def update_relay(self):
        state = self.button.isChecked()
        self.button.setChecked(state)
        self.button.setText("Close" if state else "Open")

    def precharge_routine(self, checked):
        if checked:
            self.controller.send_data("Precharge: 1")
        
    
    def update_precharge(self, state):
        if not state:
            self.precharge.setChecked(False)
    