# update_voltage_task.py
from PyQt6.QtCore import QRunnable, pyqtSignal, QObject

class UpdateVoltageTask(QRunnable):
    class Signals(QObject):
        result = pyqtSignal(int, float)

    def __init__(self, controller, cell_num, voltage):
        super(UpdateVoltageTask, self).__init__()
        self.controller = controller
        self.cell_num = cell_num
        self.voltage = voltage
        self.signals = self.Signals()

    def run(self):
        # Perform the voltage update
        self.controller.handle_change_voltage(self.cell_num, self.voltage)
        # Emit the result signal
        self.signals.result.emit(self.cell_num, self.voltage)
