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

if __name__ == '__main__':
    root           = os.path.dirname(os.path.realpath(__file__)) + '/../../'
    path           = root + 'PLAT/gccout/ec7xx_ref_1h00_ec718p/ap/ref_app/ap_ref_app.bin'

    bin_address    = 0x0087c000
    table_address  = 0x0087c200
    table_size     = 0x1000
    table_end      = table_address - bin_address + table_size
    apps_offset    = [table_end - 4]

    app0_address   = 0x00A60000
    apps_address   = [app0_address]

    update_file(path, apps_offset, apps_address)

    table_address  = 0x0087d200
    table_size     = 0x1000
    table_end      = table_address - bin_address + table_size
    ddks_offset    = [table_end - 4]

    ddk0_address   = 0x00A80000
    ddks_address   = [ddk0_address]

    update_file(path, ddks_offset, ddks_address)
