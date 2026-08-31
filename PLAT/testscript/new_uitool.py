import sys
import binascii
import os
import sys
import tkinter.filedialog
import tkinter.font
import serial.tools.list_ports
import time
import subprocess
import tempfile
import struct
import tkinter
import tkinter.ttk as ttk
import io


class UITool():
    def __init__(self) -> None:
        self.SEND_DUMMY = b'\r\n'
        self.SEND_ACK = b'\00\00\00\00\11\11\11\11'
        self.window = None
        self.ser = None
        self.port_open = False
        self.current_drive = None

    def datacrc32(self, data, lens):
        res = 0xFFFFFFFF
        res = 0
        counter = 0
        for d in data:
            # print(d.to_bytes(1,byteorder='little'))
            res = binascii.crc32(d.to_bytes(1, byteorder='little'), res)
            counter = counter+1
            if counter == lens:
                break
        return '%08x' % (res & 0xffffffff)

    def push_file(self):
        files_path = tkinter.filedialog.askopenfilenames()
        for file_path in files_path:
            if len(file_path) == 0:
                return
            str_name = os.path.basename(file_path)
            PUSH_CMD = 'get {}:/{}\r\n'.format(self.selected_drive.get(),
                                               str_name).encode()
            print("PUSH_CMD", PUSH_CMD)

            data_packet_len = 1000
            packet_size = data_packet_len+20

            localfile = open(file_path, mode='rb')
            stats = os.stat(file_path)
            file_size = stats.st_size
            print("file_size:", file_size)
            pak_count = file_size/data_packet_len
            pak_leave = file_size % data_packet_len

            print(type(pak_count))
            print(int(pak_count))

            self.ser.write(PUSH_CMD)
            time.sleep(0.1)
            s1 = self.ser.read(self.ser.in_waiting)

            print(len(s1))
            print(s1)

            str_bytes = bytes(128)
            str_bytes = str_name.encode('UTF-8')
            # send file header
            file_hdr = struct.pack('<'+'I128sII', file_size,
                                   str_bytes, packet_size, int(pak_count))
            self.ser.write(file_hdr)
            print(file_hdr)
            print(len(file_hdr))
            ack = self.ser.read(8)
            loop = int(pak_count)+1
            # send file data
            data_bytes = bytes(1000)
            pack_flag = b'\xee\xee\xee\xee'

            for i in range(loop):
                print(i)
                cur = i+1
                if cur > loop:
                    break
                print(loop)
                if cur == loop:
                    print("last")
                    file_data = localfile.read(pak_leave)
                    print(pak_leave)
                    data_bytes = file_data
                    packet_data_size = int(pak_leave)
                    packet_reserve = b'\00\00'
                    datacrc = self.datacrc32(data_bytes, pak_leave)
                    headcrc = b'\00\00\00\00'
                    print("calc data crc:", datacrc)
                    print(type(datacrc))
                    datacrc_ba = bytearray.fromhex(datacrc)
                    print(datacrc_ba)
                    file_hdr = struct.pack('<'+'I4sH2s4s4s1000s', int(i), pack_flag,
                                           packet_data_size, packet_reserve, datacrc_ba, headcrc, data_bytes)
                    print(file_hdr)
                    # file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
                    self.ser.write(file_hdr)
                else:
                    print("mid data")
                    file_data = localfile.read(data_packet_len)
                    print(data_packet_len)
                    data_bytes = file_data
                    packet_data_size = int(data_packet_len)
                    packet_reserve = b'\00\00'
                    datacrc = self.datacrc32(data_bytes, data_packet_len)
                    headcrc = b'\00\00\00\00'
                    print("calc data crc:", datacrc)
                    print(type(datacrc))
                    datacrc_ba = bytearray.fromhex(datacrc)
                    print(datacrc_ba)
                    file_hdr = struct.pack('<'+'I4sH2s4s4s1000s', int(i), pack_flag,
                                           packet_data_size, packet_reserve, datacrc_ba, headcrc, data_bytes)
                    print(file_hdr)
                    # file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
                    self.ser.write(file_hdr)
                # get data ack
                ack = self.ser.read(8)
            # send file header
            print("data over")
            self.ls_dir()
            time.sleep(0.1)

    def pull_file(self):
        drive_letter = self.selected_drive.get()
        select_files = []
        for i in self.file_list.curselection():
            select_files.append(self.file_list.get(i))
        for select_file in select_files:
            file_path = "{}:/{}".format(drive_letter,
                                        select_file[11:])
            print("pull_file_path = {}".format(file_path))
            ser = self.ser
            if drive_letter == "C":
                ser.write(b'\r\n')
                ser.write(b'\r\n')
                ser.write(b'cd C:\r\n')
                time.sleep(0.1)
                s1 = ser.read(ser.in_waiting)
            elif drive_letter == "D":
                ser.write(b'\r\n')
                ser.write(b'\r\n')
                ser.write(b'cd D:\r\n')
                time.sleep(0.1)
                s1 = ser.read(ser.in_waiting)
            PULL_CMD = 'send {}'.format(file_path).encode()
            ser.write(PULL_CMD+b"\r")
            time.sleep(0.1)
            # s1 = ser.read(cmd_lens+promt_lens+file_pkt_header_lens)
            s1 = ser.read(ser.in_waiting)
            print(len(s1))
            print(s1)
            # send c: / test.txt\n\rC:\\>
            s1 = s1.replace(PULL_CMD, b"")
            s1 = s1.replace(b"\r\n", b"")
            s1 = s1.replace(b"\n\r", b"")
            s1 = s1.replace("{}:\\>".format(
                drive_letter.upper()).encode(), b"")
            print(s1)

            filesize, file_name, packet_size, filecounter = struct.unpack(
                '<I128s2I', s1)
            # print('file_hdr:', prot)
            print('filesize:', filesize)
            real_file_name = str(file_name, 'utf8').strip('\00')
            print('file_name:', real_file_name[3:])
            print('length of file_name:', len(real_file_name))
            print('packet_size:', packet_size)
            print('filecounter:', filecounter)

            loop = filecounter + 1
            data_packet_len = packet_size-20
            localfile = open(real_file_name[3:], mode='wb')
            for i in range(loop):
                # ack
                cur = i+1
                if cur > loop:
                    break
                ser.write(self.SEND_ACK)
                s2 = ser.read(packet_size)
                print(len(s2))
                print(s2)
                # parse
                packet_counter, packet_flag, packet_data_size, packet_reserve, data_crc, header_crc, data = struct.unpack(
                    '<'+'2I2H2I'+str(data_packet_len)+'s', s2)
                print("packet_flag:%08x" % packet_flag)
                print("packet_data_size:", packet_data_size)
                print("packet_reserve:", packet_reserve)
                print("data_crc:%08x" % data_crc)
                print("header_crc:%08x" % header_crc)
                print("packet_counter:", packet_counter)
                real_data_size = packet_data_size - 20
                if cur == loop:
                    # print("calc last data crc:", datacrc32(
                    #     data, filesize % data_packet_len))
                    localfile.write(data[0:packet_data_size])
                else:
                    # print("calc data crc:", datacrc32(data, data_packet_len))
                    localfile.write(data)
                localfile.flush()
            localfile.close()
            ser.write(self.SEND_DUMMY)
            print("download ok in {}".format(real_file_name[3:]))
            download_file_ok_pop = tkinter.Toplevel(self.window)
            download_file_ok_pop.geometry("700x50")
            download_file_ok_pop.geometry(
                "+{}+{}".format(self.pop_new_x, self.pop_new_y))
            tkinter.Label(download_file_ok_pop,
                          text="download ok in {}".format(os.path.join(os.path.abspath(os.path.curdir), real_file_name[3:]))).pack(pady=10)
            # self.window.after(1000, open_com_fail_pop.destroy)

    def rm(self):
        select_files = []
        for i in self.file_list.curselection():
            select_files.append(self.file_list.get(i))
        for select_file in select_files:
            del_cmd = 'rm {}:/{}\r\n'.format(self.selected_drive.get(),
                                             select_file[11:])
            print("del_cmd = {}".format(del_cmd))
            self.ser.write(del_cmd.encode())
            time.sleep(0.1)
        self.ls_dir()

    def play(self):
        if len(self.file_list.curselection()) > 1:
            return
        play_cmd = 'play {}:/{}\r\n'.format(self.selected_drive.get(),
                                            self.file_list.get(self.file_list.curselection())[11:])
        print("play_cmd = {}".format(play_cmd))
        self.ser.write(play_cmd.encode())
        time.sleep(0.1)
        self.ls_dir()

    def stop(self):
        if len(self.file_list.curselection()) > 1:
            return
        stop_cmd = 'stop \r\n'
        print("stop_cmd = {}".format(stop_cmd))
        self.ser.write(stop_cmd.encode())
        time.sleep(0.1)
        self.ls_dir()

    def ls_dir(self):
        print("cd")
        self.ser.write(b'\r\n')
        self.ser.write(b'\r\n')
        if self.selected_drive.get() == "C":
            self.ser.write(b'cd C:\r\n')
        elif self.selected_drive.get() == "D":
            self.ser.write(b'cd D:\r\n')

        time.sleep(0.1)
        s1 = self.ser.read(self.ser.in_waiting)

        print("cd ok")

        self.ser.write(b'ls\r\n')
        time.sleep(0.6)
        s1 = self.ser.read(self.ser.in_waiting)
        # print(s1)
        s2 = s1.split(b'\n\r')
        # print(s2)

        self.file_list.delete(0, tkinter.END)

        if b'file name\tsize\r\n' in s2:
            for s in s2[s2.index(b'file name\tsize\r\n')+1:-2]:
                file_info = s.split(b'\t')
                if len(file_info) == 2:
                    row = [file_info[0].decode(), file_info[1].decode()]
                    print("{0:<10} {1}".format(row[1], row[0]))
                    self.file_list.insert(
                        tkinter.END, "{0:<10} {1}".format(row[1], row[0]))
        else:
            self.file_list.insert(
                tkinter.END, "No file found in this device")
            self.refresh_btn.configure(state="disabled")
            self.view_btn.configure(state="disabled")
            self.upload_btn.configure(state="disabled")
            self.download_btn.configure(state="disabled")
            self.rm_btn.configure(state="disabled")
            self.play_btn.configure(state="disabled")
            self.playstop_btn.configure(state="disabled")

    def view_file(self):
        if len(self.file_list.curselection()) > 1:
            return
        drive_letter = self.selected_drive.get()
        file_path = "{}:/{}".format(drive_letter,
                                    self.file_list.get(self.file_list.curselection())[11:])
        print("file_path = {}".format(file_path))
        ser = self.ser
        if drive_letter == "C":
            ser.write(b'\r\n')
            ser.write(b'\r\n')
            ser.write(b'cd C:\r\n')
            time.sleep(0.1)
            s1 = ser.read(ser.in_waiting)
        elif drive_letter == "D":
            ser.write(b'\r\n')
            ser.write(b'\r\n')
            ser.write(b'cd D:\r\n')
            time.sleep(0.1)
            s1 = ser.read(ser.in_waiting)
        PULL_CMD = 'send {}'.format(file_path).encode()
        ser.write(PULL_CMD+b"\r\n")
        time.sleep(0.1)
        # s1 = ser.read(cmd_lens+promt_lens+file_pkt_header_lens)
        s1 = ser.read(ser.in_waiting)
        print(len(s1))
        print(s1)
        # send c: / test.txt\n\rC:\\>
        s1 = s1.replace(PULL_CMD, b"")
        s1 = s1.replace(b"\r\n", b"")
        s1 = s1.replace(b"\n\r", b"")
        s1 = s1.replace("{}:\\>".format(drive_letter.upper()).encode(), b"")
        print(s1)

        filesize, file_name, packet_size, filecounter = struct.unpack(
            '<I128s2I', s1)
        # print('file_hdr:', prot)
        # print('filesize:', filesize)
        real_file_name = str(file_name, 'utf8').strip('\00')
        # print('file_name:', real_file_name[3:])
        # print('length of file_name:', len(real_file_name))
        # print('packet_size:', packet_size)
        # print('filecounter:', filecounter)

        loop = filecounter + 1
        data_packet_len = packet_size-20
        with tempfile.NamedTemporaryFile(suffix=".txt", mode='wb+', delete=False) as view_file:
            for i in range(loop):
                # ack
                cur = i+1
                if cur > loop:
                    break
                ser.write(self.SEND_ACK)
                s2 = ser.read(packet_size)
                # print(len(s2))
                # print(s2)
                # parse
                packet_counter, packet_flag, packet_data_size, packet_reserve, data_crc, header_crc, data = struct.unpack(
                    '<'+'2I2H2I'+str(data_packet_len)+'s', s2)
                # print("packet_flag:%08x" % packet_flag)
                # print("packet_data_size:", packet_data_size)
                # print("packet_reserve:", packet_reserve)
                # print("data_crc:%08x" % data_crc)
                # print("header_crc:%08x" % header_crc)
                # print("packet_counter:", packet_counter)
                real_data_size = packet_data_size - 20
                if cur == loop:
                    # print("calc last data crc:", datacrc32(
                    #     data, filesize % data_packet_len))
                    view_file.write(data[0:packet_data_size])
                else:
                    # print("calc data crc:", datacrc32(data, data_packet_len))
                    view_file.write(data)
                view_file.flush()
            ser.write(self.SEND_DUMMY)
            view_file.seek(0)
            file_content = view_file.read().decode(encoding="utf8", errors="ignore")
            print(file_content)
            subprocess.Popen(['notepad.exe', view_file.name])

    def list_ports(self):
        ports = serial.tools.list_ports.comports()
        ports_names = [port.name for port in ports]

        return ports_names

    def switch_com_port_state(self):
        try:
            if self.port_open == False:
                self.ser = serial.Serial(
                    self.selected_com.get(), self.selected_baud.get())
                self.com_select_list.configure(state="disabled")
                self.baud_select_list.configure(state="disabled")
                self.refresh_btn.configure(state=tkinter.NORMAL)
                self.view_btn.configure(state=tkinter.NORMAL)
                self.upload_btn.configure(state=tkinter.NORMAL)
                self.download_btn.configure(state=tkinter.NORMAL)
                self.rm_btn.configure(state=tkinter.NORMAL)
                self.play_btn.configure(state=tkinter.NORMAL)
                self.playstop_btn.configure(state=tkinter.NORMAL)
                self.current_drive = self.selected_drive.get()
                self.port_control_btn['text'] = "关闭串口"
                self.port_open = True
                self.ls_dir()
            elif self.port_open == True:
                self.ser.close()
                self.com_select_list.configure(state=tkinter.NORMAL)
                self.baud_select_list.configure(state=tkinter.NORMAL)
                self.refresh_btn.configure(state="disabled")
                self.view_btn.configure(state="disabled")
                self.upload_btn.configure(state="disabled")
                self.download_btn.configure(state="disabled")
                self.rm_btn.configure(state="disabled")
                self.play_btn.configure(state="disabled")
                self.playstop_btn.configure(state="disabled")
                self.port_control_btn['text'] = "打开串口"
                self.port_open = False
                self.file_list.delete(0, tkinter.END)

        except serial.serialutil.SerialException as se:
            print(se)
            open_com_fail_pop = tkinter.Toplevel(self.window)
            open_com_fail_pop.geometry("200x50")
            open_com_fail_pop.geometry(
                "+{}+{}".format(self.pop_new_x, self.pop_new_y))
            tkinter.Label(open_com_fail_pop,
                          text="Open com port fail").pack(pady=10)
            # self.window.after(1000, open_com_fail_pop.destroy)

    def change_drive(self, _):
        print(self.current_drive)
        print(self.selected_drive.get())
        if self.selected_drive.get() == self.current_drive:
            print("Do not need to change drive")
        else:
            self.current_drive = self.selected_drive.get()
            print("Need to change drive")
            if self.port_open == True:
                self.ls_dir()

    def update_toplevel_position(self, event):
        self.pop_new_x = self.window.winfo_x() + 50
        self.pop_new_y = self.window.winfo_y() + 50

    def select_all(self):
        self.file_list.selection_set(2, tkinter.END)

    def clear_select(self):
        self.file_list.selection_clear(2, tkinter.END)

    def create_window(self):
        self.window = tkinter.Tk()
        default_font = tkinter.font.nametofont("TkDefaultFont")
        default_font.configure(family="Cascadia Mono")
        self.window.title("文件浏览器")
        self.window.geometry('620x550')
        com_label = tkinter.Label(self.window, text='COM:')
        com_label.place(x=10, y=5)

        self.selected_com = tkinter.StringVar()
        self.com_select_list = ttk.Combobox(
            self.window, textvariable=self.selected_com, state="readonly")
        self.com_select_list.place(x=60, y=5, width=80)
        self.com_select_list["value"] = self.list_ports()
        self.com_select_list.current(0)

        baud_label = tkinter.Label(self.window, text='baud:')
        baud_label.place(x=160, y=5)

        self.selected_baud = tkinter.IntVar()
        self.baud_select_list = ttk.Combobox(
            self.window, textvariable=self.selected_baud, state="readonly")
        self.baud_select_list.place(x=210, y=5, width=80)
        self.baud_select_list["value"] = (9600, 115200, 460800, 921600)
        self.baud_select_list.current(1)

        drive_label = tkinter.Label(self.window, text='drive:')
        drive_label.place(x=310, y=5)

        self.selected_drive = tkinter.StringVar()
        self.drive_select_list = ttk.Combobox(
            self.window, textvariable=self.selected_drive, state="readonly")
        self.drive_select_list.place(x=370, y=5, width=80)
        self.drive_select_list["value"] = ("C", "D")
        self.drive_select_list.current(0)
        self.drive_select_list.bind("<<ComboboxSelected>>", self.change_drive)

        self.port_control_btn = ttk.Button(
            self.window, text='打开串口', command=self.switch_com_port_state)
        self.port_control_btn.place(x=500, y=0, width=100, height=30)

        self.file_list = tkinter.Listbox(self.window, width=80, height=10,
                                         selectmode=tkinter.MULTIPLE)
        self.file_list.place(x=10, y=50, width=450, height=480)

        self.refresh_btn = ttk.Button(self.window, text='刷新',
                                      command=self.ls_dir)
        self.refresh_btn.place(x=500, y=50, width=100, height=30)

        self.view_btn = ttk.Button(
            self.window, text='预览', command=self.view_file)
        self.view_btn.place(x=500, y=100, width=100, height=30)

        self.upload_btn = ttk.Button(
            self.window, text='上传', command=self.push_file)
        self.upload_btn.place(x=500, y=150, width=100, height=30)

        self.download_btn = ttk.Button(
            self.window, text='下载', command=self.pull_file)
        self.download_btn.place(x=500, y=200, width=100, height=30)

        self.rm_btn = ttk.Button(self.window, text='删除', command=self.rm)
        self.rm_btn.place(x=500, y=250, width=100, height=30)

        self.play_btn = ttk.Button(
            self.window, text='播放', command=self.play)
        self.play_btn.place(x=500, y=300, width=100, height=30)

        self.playstop_btn = ttk.Button(
            self.window, text='停止播放', command=self.stop)
        self.playstop_btn.place(x=500, y=350, width=100, height=30)

        self.select_all_btn = ttk.Button(
            self.window, text='全选', command=self.select_all)
        self.select_all_btn.place(x=500, y=400, width=100, height=30)

        self.clear_select_btn = ttk.Button(
            self.window, text='清除', command=self.clear_select)
        self.clear_select_btn.place(x=500, y=450, width=100, height=30)

        self.refresh_btn.configure(state="disabled")
        self.view_btn.configure(state="disabled")
        self.upload_btn.configure(state="disabled")
        self.download_btn.configure(state="disabled")
        self.rm_btn.configure(state="disabled")
        self.play_btn.configure(state="disabled")
        self.playstop_btn.configure(state="disabled")

        self.window.bind("<Configure>", self.update_toplevel_position)

        self.window.mainloop()


if __name__ == '__main__':
    UITool().create_window()
