from PyQt6.QtWidgets import QMainWindow, QFileDialog, QMessageBox
from PyQt6.QtCore import QDate, QTime, pyqtSignal, QObject
from camera_test_ui import Ui_MainWindow
import serial
import serial.tools.list_ports
import time
import sys
import re

class camera_test(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.InitWindow()
        self.port_open = False
        self.baudrate = 115200
        self.ser = None
        self.camera_start = False

    def InitWindow(self):
        self.setWindowTitle("相机测试工具")
        self.cbScene.clear()
        items = ["白天", "夜晚"]
        self.cbScene.addItems(items)
        self.cbFlip.clear()
        items = ["正常", "镜像", "翻转", "镜像翻转"]
        self.cbFlip.addItems(items)
        self.cbAwb.clear()
        items = ["自动", "阴天", "日光", "白炽灯", "荧光灯", "钨丝灯"]
        self.cbAwb.addItems(items)
        self.cbAwb.setCurrentIndex(0)
        self.cbAwb.currentIndexChanged.connect(self.on_Awb_changed)
        self.cbAe.clear()
        items = ["打开", "关闭"]
        self.cbAe.addItems(items)
        self.cbAe.setCurrentIndex(0)
        self.cbAe.currentIndexChanged.connect(self.on_Ae_changed)
        self.on_Refresh_clicked()
        self.pbCommOpen.clicked.connect(self.on_CommOpen_clicked)
        self.pbRefresh.clicked.connect(self.on_Refresh_clicked)
        self.pbStart.clicked.connect(self.on_Start_clicked)
        self.pbSnap.clicked.connect(self.on_Snap_clicked)
        self.pbWriteReg.clicked.connect(self.on_WriteReg_clicked)
        self.pbReadReg.clicked.connect(self.on_ReadReg_clicked)
        self.cbScene.currentIndexChanged.connect(self.on_Scene_changed)
        self.cbFlip.currentIndexChanged.connect(self.on_Flip_changed)
        self.sliderEv.setMinimum(0)
        self.sliderEv.setMaximum(100)
        self.sliderEv.setValue(50)
        self.labelEv.setText("50")
        self.sliderEv.valueChanged.connect(self.on_Ev_value_changed)
        self.sliderContrast.setMinimum(0)
        self.sliderContrast.setMaximum(100)
        self.sliderContrast.setValue(50)
        self.labelContrast.setText("50")
        self.sliderContrast.valueChanged.connect(self.on_Contrast_value_changed)
        self.sliderSat.setMinimum(0)
        self.sliderSat.setMaximum(100)
        self.sliderSat.setValue(50)
        self.labelSat.setText("50")
        self.sliderSat.valueChanged.connect(self.on_Saturation_value_changed)
        self.sliderSharp.setMinimum(0)
        self.sliderSharp.setMaximum(100)
        self.sliderSharp.setValue(50)
        self.labelSharp.setText("50")
        self.sliderSharp.valueChanged.connect(self.on_Sharp_value_changed)
        self.sliderFps.setMinimum(0)
        self.sliderFps.setMaximum(25)
        self.sliderFps.setValue(25)
        self.lableFps.setText("25")
        self.sliderFps.valueChanged.connect(self.on_Fps_value_changed)

    def on_Ae_changed(self):
        try:
            ae_mode = self.cbAe.currentIndex()
            string_data = f"camAe set {ae_mode}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机曝光模式错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")        

    def on_Awb_changed(self):
        try:
            awb_mode = self.cbAwb.currentIndex()
            string_data = f"camAwb set {awb_mode}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机白平衡模式错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def on_Sharp_value_changed(self):
        try:
            sharp_value = self.sliderSharp.value()
            string_data = f"camSharp set {sharp_value}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机锐利度错误")
            else:
                self.labelSharp.setText(str(sharp_value))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")    
    
    def on_Fps_value_changed(self):
        try:
            fps_value = self.sliderFps.value()
            string_data = f"camFps set {fps_value * 100}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机帧率错误")
            else:
                self.lableFps.setText(str(fps_value))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}") 

    def on_Saturation_value_changed(self):
        try:
            sat_value = self.sliderSat.value()
            string_data = f"camSaturation set {sat_value}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机色彩饱和度错误")
            else:
                self.labelSat.setText(str(sat_value))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")        

    def on_Contrast_value_changed(self):
        try:
            con_value = self.sliderContrast.value()
            string_data = f"camContrast set {con_value}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机对比度错误")
            else:
                self.labelContrast.setText(str(con_value))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")
    
    def on_Ev_value_changed(self):
        try:
            ev_value = self.sliderEv.value()
            string_data = f"camEv set {ev_value}\n"
            data_to_send = string_data.encode('utf-8')
            self.ser.write(data_to_send)
            self.show_send_str(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机曝光补偿错误")
            else:
                self.labelEv.setText(str(ev_value))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def show_send_str(self, str):
        str_to_show = f"serial send: {str}"
        self.teMsg.append(str_to_show)

    def on_Flip_changed(self):
        try:
            flip_mode = self.cbFlip.currentIndex()
            string_data = f"camFlip set {flip_mode}\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机翻转模式错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")


    def on_Scene_changed(self):
        try:
            scene_mode = self.cbScene.currentIndex()
            string_data = f"camScene set {scene_mode}\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret != 0):
                self.teMsg.append("配置相机场景模式错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def list_ports(self):
        ports = serial.tools.list_ports.comports()
        ports_names = [port.name for port in ports]
        return ports_names

    def on_CommOpen_clicked(self):
        try:
            if self.port_open == False:
                self.ser = serial.Serial(self.cbSerial.currentText(), self.baudrate)
                self.pbCommOpen.setText("关闭串口")
                self.port_open = True
                self.pbStart.setEnabled(True)
            elif self.port_open == True:
                self.ser.close()
                self.pbCommOpen.setText("打开串口")
                self.port_open = False
                self.pbStart.setEnabled(False)
        except serial.serialutil.SerialException as se:
            self.teMsg.append(f"打开串口失败, reason:{se}")


    def on_Refresh_clicked(self):
        self.cbSerial.clear()
        post_names = self.list_ports()
        self.cbSerial.addItems(post_names)
        self.cbSerial.setCurrentIndex(0)

    def on_Start_clicked(self):
        try:
            if(self.camera_start):
                string_data = "camstoppre\n"
                data_to_send = string_data.encode('utf-8')
                self.ser.write(data_to_send)
                self.show_send_str(data_to_send)
                time.sleep(0.3)
                s1 = self.ser.read(self.ser.in_waiting)
                self.teMsg.append(s1.decode('utf-8', errors='ignore'))
                ret, value = self.parse_cmd_string(s1)
                if(ret == 0):
                    self.teMsg.append("预览相机关闭")
                    self.camera_start = False
                    self.pbStart.setText("打开预览")
                else:
                    self.teMsg.append("关闭预览相机失败")

            else:
                string_data = "camstartpre\n"
                data_to_send = string_data.encode('utf-8')
                self.ser.write(data_to_send)
                self.show_send_str(data_to_send)
                time.sleep(0.3)
                s1 = self.ser.read(self.ser.in_waiting)
                self.teMsg.append(s1.decode('utf-8', errors='ignore'))
                ret, value = self.parse_cmd_string(s1)
                if(ret == 0):
                    self.teMsg.append("预览相机成功")
                    self.camera_start = True
                    self.pbStart.setText("关闭预览")
                    self.get_cmos_model()
                    self.get_resolution() 
                else:
                    self.teMsg.append("预览相机失败")
            
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")


    def on_Snap_clicked(self):
        self.ser.write("camshoot\n")
        time.sleep(0.3)
        s1 = self.ser.read(self.ser.in_waiting)
        self.teMsg.append(s1)

    def on_WriteReg_clicked(self):
        try:
            reg_addr = self.leRegAddr.text()
            reg_value = self.leRegValue.text()
            string_data = f"camWrReg {reg_addr} {reg_value}\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")
    
    def on_ReadReg_clicked(self):
        try:
            reg_addr = self.leRegAddr.text()
            string_data = f"camRdReg {reg_addr}\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret == 0):
                self.leRegValue.setText(value)
            else:
                self.teMsg.append("读取Sensor寄存器错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def get_cmos_model(self):
        try:
            string_data = f"camCmosModel get\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret == 0):
                if(value == "232A"):
                    self.labelSnsModel.setText("GC023A")
                elif (value == "6153"):
                    self.labelSnsModel.setText("GC6153")
                elif (value == "BA"):
                    self.labelSnsModel.setText("GC6133")
                elif (value == "3B02"):
                    self.labelSnsModel.setText("BF30A2")
                elif (value == "20A6"):
                    self.labelSnsModel.setText("BF20A6")
                else:
                    self.labelSnsModel.setText("UNKNOWN")
            else:
                self.teMsg.append("获得CMOS型号错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def get_resolution(self):
        try:
            string_data = f"camResolution get\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret == 0):
                str_resolution = f"{(int(value, 16) & 0xFFFF0000) >> 16}x{int(value, 16) & 0x0000FFFF}"
                self.labelResolution.setText(str_resolution)
            else:
                self.teMsg.append("获得图像帧率错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")
    
    def get_max_fps(self):
        try:
            string_data = f"camMaxFps get\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret == 0):
                fps = int(int(value) / 100)
                self.sliderFps.setMinimum(1)
                self.sliderFps.setMaximum(fps)
            else:
                self.teMsg.append("获得图像最大帧率错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def get_cur_fps(self):
        try:
            string_data = f"camFps get\n"
            data_to_send = string_data.encode('utf-8')
            self.show_send_str(data_to_send)
            self.ser.write(data_to_send)
            time.sleep(0.3)
            s1 = self.ser.read(self.ser.in_waiting)
            self.teMsg.append(s1.decode('utf-8', errors='ignore'))
            ret, value = self.parse_cmd_string(s1)
            if(ret == 0):
                fps = int(int(value) / 100)
                self.sliderFps.setValue(fps)
                self.lableFps.setText(str(fps))
            else:
                self.teMsg.append("获得图像帧率错误")
        except serial.SerialException as e:
            self.teMsg.append(f"串口通信出错: {e}")
        except Exception as e:
            self.teMsg.append(f"发生其他错误: {e}")

    def parse_cmd_string(self, data_str):
        pattern =  r"ret:\s*(\d+), value:\s*(.*)"
        str_to_match = data_str.decode('utf-8')
        match = re.search(pattern, str_to_match)
        if match:
            ret = int(match.group(1))
            value = match.group(2)
            translator = str.maketrans('', '', '\r\n ')
            value = value.translate(translator)
            return ret, value
        return None
    
