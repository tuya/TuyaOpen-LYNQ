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
    if len(sys.argv) != 2:
        print("Missing Bin path parameter.")
        return

    root         = os.path.dirname(os.path.realpath(__file__)) + '/../..'
    speaker_path = root + '/PLAT/' + sys.argv[1]

    bin_address   = 0x008BA000
    table_address = 0x008BA200
    table_size    = 0x1000
    table_end     = table_address - bin_address + table_size
    apps_offset   = [table_end - 4]

    app0_address  = 0x00B81000
    apps_address  = [app0_address]

    update_file(speaker_path, apps_offset, apps_address)

if __name__ == '__main__':
    file_name = os.path.basename(__file__)
    print(file_name, 'begin')
    main()
    print(file_name, 'end')
