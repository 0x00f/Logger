__author__ = 'Michael A. Schulze'
__version__ = "1.0.1"

import sys
import serial
from time import strftime  # to get system time
from PyQt4 import QtCore, QtGui, uic
import struct

form_class = uic.loadUiType("STM32F4_Datenlogger.ui")[0]                 # Load the UI

# Find all currently connected USB-Serial Converters
try:
    from serial.tools.list_ports import comports
    # print(comports)
except ImportError:
    comports = None

class MyWindowClass(QtGui.QMainWindow, form_class):
    def __init__(self, parent=None):
        QtGui.QMainWindow.__init__(self, parent)
        self.setupUi(self)
        self.dump_port_list()

        self.sample_rates = ['250', '500', '1000', '2000', '4000', '5000', '8000', '10000', '12000',
                             '16000', '20000', '32000', '64000']

        self.baud_rate = 921600
        self.serial_number = 0

        self.serial_connection = serial.Serial()

        self.col = QtGui.QColor(0, 0, 0)
        self.connStatusBox.setStyleSheet("QWidget { background-color: %s }" % self.col.name())

        for s in self.sample_rates:
            self.sampleRateComboBox.addItem(s)
        self.sampleRateComboBox.activated[str].connect(self.sampleRateSelected)

        self.adc_ch_checkboxes = []
        self.adc_ch_checkboxes.append(self.ADC1_CH0_chkbx)
        self.adc_ch_checkboxes.append(self.ADC1_CH1_chkbx)
        self.adc_ch_checkboxes.append(self.ADC1_CH2_chkbx)
        self.adc_ch_checkboxes.append(self.ADC1_CH3_chkbx)
        self.adc_ch_checkboxes.append(self.ADC2_CH0_chkbx)
        self.adc_ch_checkboxes.append(self.ADC2_CH1_chkbx)
        self.adc_ch_checkboxes.append(self.ADC2_CH2_chkbx)
        self.adc_ch_checkboxes.append(self.ADC2_CH3_chkbx)

        self.detectDevBtn.clicked.connect(self.device_detect_clicked)
        self.connectButton.clicked.connect(self.connect_to_logger)
        self.getRtcButton.clicked.connect(self.get_rtc_time)
        self.syncRtcButton.clicked.connect(self.set_rtc_time)
        self.getVoltageButton.clicked.connect(self.get_backup_voltage)
        self.saveConfigButton.clicked.connect(self.set_filename)
        self.readConfigButton.clicked.connect(self.get_filename)

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.Time)
        self.timer.start(10)



# -----------------------------------Functions--------------------------------------------

    def device_detect_clicked(self):
        self.find_logger_port()

    def get_rtc_time(self):
        self.serial_connection.flush()
        self.serial_connection.write("GET_RTC\n".encode())
        time = self.serial_connection.readline()
        print("Received %s\n" % time)
        self.deviceTimeText.setText(time.decode())

    def set_rtc_time(self):

        now_dmy = strftime("%d"+"."+"%m"+"."+"%y"+".")
        now_dow = strftime("%w")
        now_time = strftime("."+"%H"+"."+"%M"+"."+"%S")
        if now_dow == "0":  # The STM RTC Library requires Sunday to be 7, not 0
            now_dow = "7"
        print("time: ", now_dmy + now_dow + now_time)
        self.serial_connection.flush()
        self.serial_connection.write(("SET_RTC "+now_dmy + now_dow + now_time).encode())
        time = self.serial_connection.readline()
        print("Received %s\n" % time)
        self.deviceTimeText.setText(time.decode())

    def set_filename(self):
        filename = self.fileNameText.text()
        print("New filename: ", self.fileNameText.text())
        self.serial_connection.flush()
        self.serial_connection.write(("SET_FILENAME " + self.fileNameText.text()+'\n').encode())
        new_filename = self.serial_connection.readline()
        print("Received %s\n" % new_filename)
        self.fileNameText.setText(new_filename[3])

    def get_filename(self):
        self.serial_connection.flush()
        self.serial_connection.write("GET_FILENAME\n".encode())
        filename = self.serial_connection.readline()
        filename_string = filename.decode().split(" ")
        print("Received %s\n" % filename_string)
        self.fileNameText.setText(filename_string[2])

    def get_backup_voltage(self):
        self.serial_connection.flush()
        self.serial_connection.write("GET_VBK\n".encode())
        voltage = self.serial_connection.readline()
        print("Received %s\n" % voltage)
        self.backupVoltageText.setText(voltage.decode())


    def sampleRateSelected(self):
        if self.sampleRateComboBox.currentText() in ("250", "500", "1000", "2000", "5000"):
            print(self.sampleRateComboBox.currentText())
            for chk in self.adc_ch_checkboxes:
                chk.setChecked(False)
            for chk in self.adc_ch_checkboxes:
                chk.setChecked(True)

        elif self.sampleRateComboBox.currentText() in ("8000", "10000", "12000"):
            print(self.sampleRateComboBox.currentText())
            for chk in self.adc_ch_checkboxes:
                chk.setChecked(False)
            i = 0
            for chk in self.adc_ch_checkboxes:
                if i < 4:
                    chk.setChecked(True)
                    i += 1
                else:
                    chk.setChecked(False)
                    i += 1

        elif self.sampleRateComboBox.currentText() in ("16000", "20000", "32000", "64000"):
            print(self.sampleRateComboBox.currentText())
            for chk in self.adc_ch_checkboxes:
                chk.setChecked(False)
            i = 0
            for chk in self.adc_ch_checkboxes:
                if i < 1:
                    chk.setChecked(True)
                    i += 1
                else:
                    chk.setChecked(False)
                    i += 1



    def dump_port_list(self):
        if comports:
            sys.stderr.write('\n--- Available serial ports:\n')
            if comports is None:
                sys.stderr.write('No COM Ports found')
            for port, desc, hwid in sorted(comports()):
                sys.stderr.write('--- %-20s %s [%s]\n' % (port, desc, hwid))


    def find_logger_port(self):
        logger_list = []
        if comports:
            sys.stderr.write('\n--Searching for DataLogger Serial Number: \n')
            if comports is None:
                sys.stderr.write('No COM Ports found')
            for port, desc, hwid in sorted(comports()):
                sys.stderr.write('--- %-20s %s [%s]\n' % (port, desc, hwid))
                x = hwid.find("ZARM")
                if x > 0:
                    print('ZARM Logger device found on port: %s\n' % port)
                    self.comPortText.setText(str(port))
                    self.serial_number = hwid[x:x+9]
                    logger_list.append((port, hwid))

                    # print(logger_list)
                else:
                    print('No ZARM Logger devices found\n')
                    self.comPortText.setText("No HW found")

    def connect_to_logger(self):
        if self.connectButton.text == "Disconnect":
            if self.serial_connection.isOpen():
                self.connectButton.setText("Connect")
                self.serial_connection.close()
        else:
            self.serial_connection = serial.Serial(self.comPortText.text(), self.baud_rate, timeout=0.005)
            if self.serial_connection.isOpen():
                self.connectButton.setText("Disconnect")

                print("Serial connection established")
                self.col = QtGui.QColor(0, 255, 0)  # Green
                self.connStatusBox.setStyleSheet("QWidget { background-color: %s }" % self.col.name())
                self.serial_connection.flush()
                self.serial_connection.write("GET_FWV\n".encode())
                firmware = self.serial_connection.readline()
                print("Received %s\n" % firmware)
                self.fwText.setText(firmware.decode())
                self.serialText.setText(self.serial_number)
            else:
                print("Could not connect to Logger")
                self.col = QtGui.QColor(255, 0, 0)  # Red
                self.connStatusBox.setStyleSheet("QWidget { background-color: %s }" % self.col.name())


# -----------------------------------------Slots----------------------------------------

    def Time(self):
        # get current date and time
        now = QtCore.QDateTime.currentDateTime()
        # set current date and time to the object
        self.lineEditSystemTime.setText(strftime("%d"+"."+"%m"+"."+"%y"+" "+"%H"+":"+"%M"+":"+"%S"))

app = QtGui.QApplication(sys.argv)
myWindow = MyWindowClass(None)
myWindow.show()
app.exec_()
