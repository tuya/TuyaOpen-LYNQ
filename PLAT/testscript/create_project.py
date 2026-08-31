import os
import sys
import shutil


def copy_dir(list_copy, name_src, name_dst):
    for path in list_copy:
        src = path % (name_src)
        dst = path % (name_dst)
        if os.path.isdir(dst):
            shutil.rmtree(dst)
        if os.path.isdir(src):
            shutil.copytree(src, dst)
        else:
            print('Error: ' + src + ' is not exists.')

def rename_path(list_rename, name_src, name_dst):
    if (name_src == name_dst):
        return 0

    for path in list_rename:
        src = path % (name_src)
        dst = path % (name_dst)
        if os.path.isdir(dst):
            shutil.rmtree(dst)
        if os.path.exists(src):
            os.rename(src, dst)
        else:
            print('Error: ' + src + ' is not exists.')

def update_file(list_update, list_src, list_dst):
    for path in list_update:
        if os.path.exists(path):
            file    = open(path, mode='r+')
            content = file.read()
            for i in range(len(list_src)):
                content = content.replace(list_src[i], list_dst[i])
            file.seek(0)
            file.truncate(0)
            file.write(content)
            file.close()
        else:
            print('Error: ' + path + ' is not exists.')

if __name__ == '__main__':
    root                = os.path.dirname(os.path.realpath(__file__)) + '/../../'
    board_src           = 'ec7xx_ref_1h00'
    board_dst           = 'ec7xx_aikit_1h00'
    project_src         = 'audio_demo'
    project_dst         = 'audio_demo'
    target_src          = 'AVAILABLE_TARGETS = ' + board_src
    target_dst          = 'AVAILABLE_TARGETS = ' + board_dst

    list_copy           = [root + 'PLAT/device/target/board/%s',
                           root + 'PLAT/driver/board/%s',
                           root + 'PLAT/project/%s/ap/apps/bootloader',
                           root + 'PLAT/project/%s/ap/apps/'  + project_src]
    list_rename_board   = [root + 'PLAT/device/target/board/' + board_dst + '/ap/%s_ap.mk',
                           root + 'PLAT/device/target/board/' + board_dst + '/ap/gcc/%s_flash.ld',
                           root + 'PLAT/device/target/board/' + board_dst + '/ap/gcc/ec718xm/%s_flash.ld',
                           root + 'PLAT/project/' + board_dst + '/ap/apps/bootloader/GCC/%s_flash.ld',
                           root + 'PLAT/project/' + board_dst + '/ap/apps/bootloader/GCC/ec718xm/%s_flash.ld']
    list_rename_project = [root + 'PLAT/project/' + board_dst + '/ap/apps/%s']
    list_update_file    = [root + 'PLAT/project/' + board_dst + '/ap/apps/' + project_dst + '/GCC/Makefile',
                           root + 'PLAT/project/' + board_dst + '/ap/apps/bootloader/GCC/Makefile']

    copy_dir(list_copy, board_src, board_dst)
    rename_path(list_rename_board,   board_src,    board_dst)
    rename_path(list_rename_project, project_src,  project_dst)
    update_file(list_update_file,    [target_src], [target_dst])
    print('End')
