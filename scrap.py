import sys

from PyQt6 import QtCore, QtGui
from PyQt6.QtWidgets import (
    QWidget,
    QApplication,
    QMainWindow,
    QCheckBox,
    QFormLayout,
    QFrame,
    QLabel,
    QLineEdit,
    QWidget,
    QVBoxLayout,
    QComboBox,
    QPushButton,
    QLabel
)

from widgets import Slider

class Voltages(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Cell Voltages")
        self.resize(750,750)
        
        self.control_label = QLabel(text = "Master Control")
        self.checkbox = QCheckBox()

        layout = QFormLayout()
        # Add widgets to the layout
        layout.addRow(self.control_label, self.checkbox)
        # Set the layout on the application's window
        self.setLayout(layout)

        #OFF Frame
        self.module_label = QLabel(text = "Module")
        self.module = QComboBox()
        self.module.addItems(['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25'])
        self.cell_title = QLabel(text = "Cell Voltages")

        self.test_slider2 = Slider(self)
        self.test_slider2.make_slider(("Cell 1"), l)

        self.test_slider3 = Slider(self)
        self.test_slider3.make_slider(("   Liczba\n   czegos"), 230, 95, 6, 1, 12, Qt.Vertical)

        self.test_slider4 = Slider(self)
        self.test_slider4.make_slider(("   Liczba\n   czegos"), 320, 95, 6, 1, 12, Qt.Vertical)

        self.test_slider5 = Slider(self)
        self.test_slider5.make_slider(("   Liczba\n   czegos"), 410, 95, 6, 1, 12, Qt.Vertical)

        self.bumping_slider = Slider(self)
        self.bumping_slider.make_slider(("     Kołek\nbumpingowy"), 500, 95, 10, 1, 12, Qt.Vertical)

        self.column_slider = Slider(self)
        self.column_slider.make_slider(("   Liczba\n   kolumn"), 590, 95, 6, 1, 12, Qt.Vertical)

        self.test_slider = Slider(self)
        self.test_slider.make_slider(("   Liczba\n   czegos"), 680, 95, 6, 1, 12, Qt.Vertical)

        self.off_frame = QFrame()
        off_layout = QFormLayout()
        off_layout.addRow(self.module_label, self.module)
        off_layout.addRow(self.cell_title)

        self.off_frame.setLayout(off_layout)

        layout.addRow(self.off_frame)

        #ON Frame
        self.all_voltage_label = QLabel("Set All Voltages")
        self.all_voltage = QLineEdit()
        
        self.on_frame = QFrame()
        on_layout = QFormLayout()
        on_layout.addRow(self.all_voltage_label, self.all_voltage)

        self.on_frame.setLayout(on_layout)
        layout.addRow(self.on_frame)

        self.checkbox.checkStateChanged.connect(self.checkbox_is_checked)



    def checkbox_is_checked(self):
        if self.checkbox.isChecked():
            self.off_frame.hide()
            self.on_frame.show()
        else:
            self.off_frame.show()
            self.on_frame.hide()


class Temperatures(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Cell Temperatures")

        # buttons
        self.control_label = QLabel(text = "Master Control")
        self.checkbox = QCheckBox()
        self.all_temp_lbl = QLabel(text = "Set All Temperatures")
        self.all_temp = QLineEdit()
        self.all_temp.setMaxLength(2)
        self.module_label = QLabel(text = "Module")
        self.module = QComboBox()
        self.module.addItems(['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25'])

        layout = QFormLayout()
        # Add widgets to the layout
        layout.addRow(self.control_label, self.checkbox)
        layout.addRow(self.all_temp_lbl, self.all_temp)
        layout.addRow(self.module_label, self.module)
        # Set the layout on the application's window
        self.setLayout(layout)
    
class Relays(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Relay Controls")

        self.breaktor_label = QLabel("Breaktor")
        self.breaktor = QComboBox()
        self.breaktor.addItems(['On', 'Off'])

        self.relay1_label = QLabel("Relay 1 (+)")
        self.relay1 = QComboBox()
        self.relay1.addItems(['On', 'Off'])

        self.relay2_label = QLabel("Relay 2 (+)")
        self.relay2 = QComboBox()
        self.relay2.addItems(['On', 'Off'])

        self.relay3_label = QLabel("Relay 3 (-)")
        self.relay3 = QComboBox()
        self.relay3.addItems(['On', 'Off'])

        self.relay4_label = QLabel("Relay 4 (-)")
        self.relay4 = QComboBox()
        self.relay4.addItems(['On', 'Off'])

        layout = QFormLayout()
        layout.addRow(self.breaktor_label, self.breaktor)
        layout.addRow(self.relay1_label, self.relay1)
        layout.addRow(self.relay2_label, self.relay2)
        layout.addRow(self.relay3_label, self.relay3)
        layout.addRow(self.relay4_label, self.relay4)

        self.setLayout(layout)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BMS Parameters")
        self.resize(400, 300)
        self.window1 = Voltages()
        self.window2 = Temperatures()
        self.window3 = Relays()

        l = QVBoxLayout()
        button1 = QPushButton("Voltages")
        button1.resize(100,60)
        button1.clicked.connect(
            lambda checked: self.toggle_window(self.window1)
        )
        l.addWidget(button1)

        button2 = QPushButton("Temperatures")
        button2.resize(100,60)
        button2.clicked.connect(
            lambda checked: self.toggle_window(self.window2)
        )
        l.addWidget(button2)

        button3 = QPushButton("Relays")
        button3.resize(100,60)
        button3.clicked.connect(
            lambda checked: self.toggle_window(self.window3)
        )
        l.addWidget(button3)

        w = QWidget()
        w.setLayout(l)
        self.setCentralWidget(w)

    def toggle_window(self, window):
        if window.isVisible():
            window.hide()

        else:
            window.show()

if __name__ == "__main__":
    app = QApplication([])
    window = MainWindow()
    window.show()
    app.exec()
