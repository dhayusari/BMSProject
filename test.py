def lengthOfLongestSubstring(s: str) -> int:
    right = 1
    left = 0
    
    while right < len(s) - 1:
        temp = s[right]
        print("right: ", s[right])
        print("left:", s[left])
        if s[right] != temp:
            left+= 1
        else:
            right+=1
    
    print(right + 1 - left)
    return right + 1 - left

def findMedianSortedArrays( nums1: [int], nums2: [int]) -> float:
    sum = len(nums1) + len(nums2)
    merged = []
    median_i = int(sum/2)
    i,j = 0, 0

    if not nums1:
        merged.extend(nums2)
    elif not nums2:
        merged.extend(nums1)
    else:
        for count in range(median_i + 1):
            if i >= len(nums1):
                merged.append(nums2[j])
                j += 1
            elif j >= len(nums2):
                merged.append(nums1[i])
                i += 1
            elif nums1[i] < nums2[j]:
                merged.append(nums1[i])
                i += 1
            else:
                merged.append(nums2[j])
                j += 1
    print(merged)
    if ((sum % 2) == 0):
        return (merged[median_i - 1] + merged[median_i]) / 2
    else:
        return merged[median_i]



out = findMedianSortedArrays([1], [2, 3, 4])

import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QLineEdit, QTextEdit, QGridLayout, QLabel, QPushButton, QWidget, QHBoxLayout, QScrollArea
from PyQt6.QtCore import Qt, QTimer
import serial

class SerialMonitor(QMainWindow):
    def _init_(self):
        super()._init_()

        self.setWindowTitle("Serial Monitor")
        self.setGeometry(100, 100, 600, 400)

        self.serial_port = serial.Serial('com4', 115200, timeout = 1)

        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        self.text_area = QTextEdit()  #Add Display Box
        self.text_area.setReadOnly(True)
        layout.addWidget(self.text_area)

        self.input_field = QLineEdit() #Add input Box
        self.input_field.returnPressed.connect(self.send_data)
        layout.addWidget(self.input_field)

        buttons_layout = QHBoxLayout() #Buttons layout

        self.send_button = QPushButton("Send") #Add a Send Button
        self.send_button.clicked.connect(self.send_data)
        layout.addWidget(self.send_button)

        self.clear_button = QPushButton("Clear") #Add a Clear Button
        self.clear_button.clicked.connect(self.clear_text_area)
        buttons_layout.addWidget(self.clear_button)

        self.timer = QTimer() #Add Timer to read Data
        self.timer.timeout.connect(self.read_data)
        self.timer.start(100)

        layout.addLayout(buttons_layout) # Add the buttons to main layout

        container = QWidget() 
        container.setLayout(layout)
        self.setCentralWidget(container)

    def send_data(self):  #Add Logic for sending data
        data = self.input_field.text()
        if data:
            self.serial_port.write((data + '\n').encode('utf-8'))
            self.input_field.clear()
    
    def read_data(self): #Add Logic to read Data
        if self.serial_port.in_waiting > 0:
            data = self.serial_port.readline().decode('utf-8').strip()           
            self.text_area.append(data)

    def clear_text_area(self): #Clear the text
        self.text_area.clear()

    def closeEvent(self, event):
        self.serial_port.close()

if _name_ == '_main_':
    app = QApplication(sys.argv)
    window = SerialMonitor()
    window.show()
    sys.exit(app.exec())


import re
import serial

class Data:
 
    def __init__(self):
 
        self.voltages = [0.0] * 200
        self.temps = [0] * 50
 
    def change_voltage(self, cell_num, volt): 
        print("Changed Cell Voltage:", cell_num) 
        self.voltages[cell_num - 1] = volt 
 
    def set_range_voltage(self, start, end, volt): 
        print("Change Voltage Cell Range") 
        print("Start Range: ", start) 
        print("End Range: ", end) 
 
        for i in range(start - 1, end):

            print("Changed Cell Voltage:", i + 1) 
            self.voltages[i] = volt 

    def change_temp(self, temp_num, temp): 
 
        print("Changed Temp Number: ", temp_num) 
        self.temps[temp_num - 1] = temp

    def change_pots(self, pot_num, pot):
 
        print("Changed Pot Value {pot_num}: ", pot_num) 
        self.pots[pot_num - 1] = pot

    def update_from_serial_data(self, data):

        pattern1 = r"Cell(\s([\d.]+):\s*([\d.]+)"

        pattern2 = r"Temp(\s([\d.]+):\s*([\d.]+)"

        pattern3 = r"Pot1:\s*(\d+),\s*pot2:/s*(\d+),\s*pot3:/s*(\d+),\s*pot4:/s*(\d+),\s*pot5:/s*(\d+),\s*pot6:/s*(\d+),\s*pot7:/s*(\d+),\s*pot8:/s*(\d+),\s*pot9:/s*(\d+),\s*pot10:/s*(\d+),\s*pot11:/s*(\d+),\s*pot12:/s*(\d+),\s*pot13:/s*(\d+),\s*pot14:/s*(\d+)"

        cell_match = re.search(pattern1, data) 

        temp_match = re.search(pattern2, data)

        pots_match = re.search(pattern3, data) 

        if cell_match: 
            cell_num = int(cell_match.group(1))
            cell_value = float(cell_match.group(2))
            self.change_voltage(1, cell_num - 1, cell_value)
        if temp_match:
            temp_num = int(temp_match.group(1))
            temp_value = float(temp_match.group(2))
            self.change_temp(1, temp_num - 1, temp_value)
        if pots_match:
            pots_values = float(pots_match.group(1))
            for i, pot_value in enumerate(pots_values, start = 1):
                self.change_pots(i, int(pot_value))
        ser = serial.Serial(port = 'COM3')
        
        try:
            while True:
                if ser.in_waiting > 0:
                    data = ser.readline().decode('utf-8').strip()
                    print(f"Received: {data}")
                    bms_data.update_from_serial_data(data)
        finally:
            ser.close()