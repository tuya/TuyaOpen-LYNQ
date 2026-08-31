import sys
import re
import struct
from PIL import Image
import io
import os

def list_c_files(directory):
    for file in os.listdir(directory):
        if file.endswith(".c"):
            print(file)
            new_c_file_path = file
            with open(new_c_file_path, 'r') as file:
                new_c_file_full_content = file.read()

            c_source_code = new_c_file_full_content
            # Regular expression pattern to match lv_img_dsc_t structures
            lv_img_dsc_pattern = re.compile(r'lv_img_dsc_t\s+(\w+)\s*=\s*{[^}]*};')

            # Find all matches in the source code
            lv_img_dsc_matches = lv_img_dsc_pattern.findall(c_source_code)

            print(lv_img_dsc_matches)
            # To extract the resolution and data array names for each lv_img_dsc_t structure,
            # we need to parse the C source code. Each structure should have fields like 'header.cf',
            # 'header.w', 'header.h', and a 'data' field pointing to an array.

            # Regular expression patterns to match the resolution and data array names
            resolution_pattern = re.compile(r'lv_img_dsc_t\s+(\w+)\s*=\s*{[^}]*header\.w\s*=\s*(\d+),[^}]*header\.h\s*=\s*(\d+),')
            data_type_pattern = re.compile(r'lv_img_dsc_t\s+(\w+)\s*=\s*{[^}]*header\.cf\s*=\s*(\w+),?')
            data_array_pattern = re.compile(r'lv_img_dsc_t\s+(\w+)\s*=\s*{[^}]*data\s*=\s*(\w+),?')


            # Find all matches for resolution and data array names
            resolution_matches = resolution_pattern.findall(c_source_code)
            data_type_matches = data_type_pattern.findall(c_source_code)
            data_array_matches = data_array_pattern.findall(c_source_code)

            # Combine the results into a list of tuples (struct_name, width, height, data_array_name)
            struct_details = []
            for (struct_name, width, height) in resolution_matches:
                # Find the corresponding data array name for the struct
                data_type_name = next((da_name for (sn, da_name) in data_type_matches if sn == struct_name), None)
                data_array_name = next((da_name for (sn, da_name) in data_array_matches if sn == struct_name), None)
                struct_details.append((struct_name, width, height, data_array_name,data_type_name))

            index = 0
            print(struct_details[index][3])
            print(struct_details[index][4])
            ##########################################################

            for a_index in range(0,len(struct_details),1):
                print(a_index)
                print(struct_details[a_index])
                array_name = struct_details[a_index][3]

                # 定位图像数据开始和结束的位置
                start_marker = array_name+'[] = {'
                end_marker = '};'

                start_index = new_c_file_full_content.find(start_marker) + len(start_marker)
                print(start_index)
                end_index = new_c_file_full_content.find(end_marker, start_index)

                # 提取图像数据
                image_data = new_c_file_full_content[start_index:end_index].strip()

                # print(image_data)
                # 使用正则表达式来找到宽度和高度
                # width_match = re.search(r'.header.w = (\d+),', new_c_file_full_content)
                # height_match = re.search(r'.header.h = (\d+),', new_c_file_full_content)

                # 提取宽度和高度值
                # width = int(width_match.group(1)) if width_match else None
                # height = int(height_match.group(1)) if height_match else None

                width = int(struct_details[index][1])
                height = int(struct_details[index][2])
                print(width, height)

                image_data_values = re.sub(r'[{}]', '', image_data).split(',')

                # 修复 RGB565 解析过程
                # 清理数据并确保所有值都是有效的十六进制数

                # 移除数据中的非数字字符，并按逗号分割
                image_data_values_clean = [value for value in image_data_values if value]

                if struct_details[index][4] == 'LV_IMG_CF_TRUE_COLOR':
                    # 重新尝试以 RGB565 格式解析图像数据
                    pixels_rgb565_clean = []

                    for i in range(0, len(image_data_values_clean), 2):
                        # 提取两个连续的字节
                        high_byte = int(image_data_values_clean[i], 16)
                        low_byte = int(image_data_values_clean[i+1], 16)
                        
                        # 提取 R, G, B 值
                        r = (high_byte & 0xF8) >> 3
                        g = ((high_byte & 0x07) << 3) | ((low_byte & 0xE0) >> 5)
                        b = low_byte & 0x1F
                        
                        # 扩展到8位
                        r = (r * 255) // 31
                        g = (g * 255) // 63
                        b = (b * 255) // 31
                        
                        # 添加到像素列表
                        pixels_rgb565_clean.append((r, g, b))

                    len_pixels_rgb565_clean = len(pixels_rgb565_clean)
                    print(len_pixels_rgb565_clean)
                    # 检查像素列表的长度是否正确
                    len_pixels_rgb565_clean, width * height

                    png_image_rgb565_clean = Image.new("RGB", (width, height))
                    png_image_rgb565_clean.putdata(pixels_rgb565_clean)

                    png_file_path_rgb565_clean = array_name+'.png'
                    png_image_rgb565_clean.save(png_file_path_rgb565_clean)

                if struct_details[index][4] == 'LV_IMG_CF_TRUE_COLOR_ALPHA':
                    # 重新尝试以 RGB565 格式解析图像数据
                    pixels_rgb565_clean = []

                    for i in range(0, len(image_data_values_clean), 3):
                        # 提取两个连续的字节
                        
                        high_byte = int(image_data_values_clean[i], 16)
                        low_byte = int(image_data_values_clean[i+1], 16)
                        alpha_byte = int(image_data_values_clean[i+2] ,16)
                        
                        # 提取 R, G, B 值
                        r = (high_byte & 0xF8) >> 3
                        g = ((high_byte & 0x07) << 3) | ((low_byte & 0xE0) >> 5)
                        b = low_byte & 0x1F
                        
                        # 扩展到8位
                        r = (r * 255) // 31
                        g = (g * 255) // 63
                        b = (b * 255) // 31
                        a = alpha_byte
                        
                        # 添加到像素列表
                        pixels_rgb565_clean.append((r, g, b, a))

                    len_pixels_rgb565_clean = len(pixels_rgb565_clean)
                    print(len_pixels_rgb565_clean)
                    # 检查像素列表的长度是否正确
                    len_pixels_rgb565_clean, width * height

                    png_image_rgb565_clean = Image.new("RGBA", (width, height))
                    png_image_rgb565_clean.putdata(pixels_rgb565_clean)

                    png_file_path_rgb565_clean = array_name+'.png'
                    png_image_rgb565_clean.save(png_file_path_rgb565_clean)
                    print('Saved')
        
current_directory = '.'
list_c_files(current_directory)