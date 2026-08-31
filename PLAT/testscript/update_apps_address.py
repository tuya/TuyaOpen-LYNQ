import os
import sys
import shutil
import struct


def update_file(path, apps_offset, apps_address):
    if os.path.exists(path):
        file    = open(path, mode='rb+')
        content = file.read()
        app_count = min(len(apps_offset), len(apps_address))
        for index in range(app_count):
            content = content[:apps_offset[index]] + struct.pack('I', apps_address[index]) + content[apps_offset[index] + 4:]
        file.seek(0)
        file.truncate(0)
        file.write(content)
        file.close()
    else:
        print('Error: ' + path + ' is not exists.')

def main():
    if len(sys.argv) != 3:
        print("Missing Bin path parameter.")
        return

    root         = os.path.dirname(os.path.realpath(__file__)) + '/../..'
    speaker_path = root + '/PLAT/' + sys.argv[1]

    if (sys.argv[2] == 'ec716e') or (sys.argv[2] == 'ec718p') or (sys.argv[2] == 'ec718pm'):
        app0_address  = 0x00B81000
    elif (sys.argv[2] == 'ec718hm'):
        app0_address  = 0x00F04000
    else:
        print('Unsupported type: ' + sys.argv[2])
        return

    app0_offset = 0x11FC
    apps_offset   = [app0_offset]
    apps_address  = [app0_address]

    update_file(speaker_path, apps_offset, apps_address)

if __name__ == '__main__':
    file_name = os.path.basename(__file__)
    print(file_name, 'begin')
    main()
    print(file_name, 'end')
