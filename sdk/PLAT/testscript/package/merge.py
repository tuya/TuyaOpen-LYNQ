import os
import argparse
import glob
import array
import struct 
from PIL import Image
import io
import re

typedef_struct = """
typedef struct {
    char        path[40];
    uint32_t    addr;
    uint32_t    size;
    uint16_t    width;
    uint16_t    height;
    uint32_t    offset;
} ext_bin_desc_t;
extern ext_bin_desc_t ext_bin_desc[];
"""

def convert_images_dir(input_dir, output_dir, max_width=240, max_height=320):
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Dictionary to keep track of the last counter for each size
    counters = {}
    
    # Iterate over all files in the input directory
    for filename in os.listdir(input_dir):
        # Check if the file is an image
        if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff')):
            file_path = os.path.join(input_dir, filename)
            try:
                # Open the image file
                with Image.open(file_path) as img:
                    # Get the dimensions of the image
                    width, height = img.size
                    
                    # Check if the image dimensions are within the specified range
                    if width > max_width or height > max_height:
                        print(f"Skipping {filename} as it exceeds the maximum dimensions ({max_width}x{max_height})")
                        continue
                    
                    # Generate a new PNG filename
                    name, ext = os.path.splitext(filename)
                    # Use the counter for this size if it exists, otherwise start at 1
                    counter = counters.get((width, height), 0) + 1
                    new_filename = f"{counter:02d}_{name}.png"
                    # Update the counter for this size
                    counters[(width, height)] = counter
                    # Convert the image to PNG and save it
                    new_file_path = os.path.join(output_dir, new_filename)
                    img.save(new_file_path, 'PNG')
                    print(f"Converted {filename} to {new_filename}")
            except Exception as e:
                print(f"Error processing {filename}: {e}")

# input_directory = './images/'  # Input directory path
# output_directory = './images/png'  # Output directory path
# convert_images_dir(input_directory, output_directory)

# Function to convert 8-bit RGB to 16-bit RGB565
def r8g8b8_to_rgb565(r, g, b):
    # Scale the 8-bit RGB values to 5-bit and 6-bit respectively
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    # Combine the 5-bit R, 6-bit G, and 5-bit B values into a 16-bit RGB565 value
    return (r5 << 11) | (g6 << 5) | b5

# Function to convert RGB565 to RGB
def rgb565_to_rgb888(rgb565):
    r = (rgb565 >> 11) & 0x1F
    g = (rgb565 >> 5) & 0x3F
    b = rgb565 & 0x1F
    return (r << 3, g << 2, b << 3) 

def rgb565_to_r8g8b8(rgb565):
    # 将RGB565值分解为RGB分量
    r5 = (rgb565 >> 11) & 0x1F
    g6 = (rgb565 >> 5) & 0x3F
    b5 = rgb565 & 0x1F

    # 将RGB565分量转换为8位RGB分量
    r = (r5 * 255 + 15) // 31
    g = (g6 * 255 + 31) // 63
    b = (b5 * 255 + 15) // 31

    return (r, g, b)

def rgb565raw_show(rgb565_data, width=240, height=320):
    # Check if the input is a list of integers
    if not all(isinstance(x, int) for x in rgb565_data):
        raise ValueError("rgb565_data must be a list of integers.")
    
    # Skip the first 4 bytes (header)
    # Since rgb565_data is already a list of integers, we don't need to read from a file
    # We can simply slice the list to skip the first 4 bytes
    rgb565_data = rgb565_data[4:]
    
    # Create a new image with the specified size
    img = Image.new('RGB', (width, height))
    
    # Iterate over each pixel in the image
    for y in range(height):
        for x in range(width):
            # Calculate the index of the current pixel in the RGB565 data
            index = y * width + x
            # Ensure the index is within the bounds of the data array
            if index < len(rgb565_data):
                rgb565 = rgb565_data[index]
                
                # Convert the RGB565 value to RGB
                r = (rgb565 & 0xF800) >> 11
                g = (rgb565 & 0x07E0) >> 5
                b = rgb565 & 0x001F
                
                # Scale the RGB values to the full 8-bit range
                r = (r * 255) // 31
                g = (g * 255) // 63
                b = (b * 255) // 31
                
                # Set the pixel in the image
                img.putpixel((x, y), (r, g, b))
    
    # Display the image
    img.show()
    # Save the image to a file (optional)
    # img.save('./buffer.png')
    return img

def swap_endianness(rgb565_data):
    # Swap the endianness of each 16-bit value
    return [struct.unpack('<H', struct.pack('>H', value))[0] for value in rgb565_data]

def convbin_to_png(bin_file_path, png_file_path, width_px=240, height_px=320, swap_endian=True):
    # Open the binary file
    with open(bin_file_path, 'rb') as bin_file:
        # Read the first 4 bytes and convert them to an integer
        header_bytes = bin_file.read(4)
        header_int = int.from_bytes(header_bytes, byteorder='big', signed=False)
        
        # Read the file content and skip the first 4 bytes
        bin_data = bin_file.read()[4:]  # Skip the first 4 bytes

    # Calculate the expected number of pixels
    expected_pixels = width_px * height_px

    # Unpack the binary data into RGB565 format
    rgb565_data = struct.unpack('>' + 'H' * (len(bin_data) // 2), bin_data)

    # Swap endianness if required
    if swap_endian:
        rgb565_data = swap_endianness(rgb565_data)

    # Create a new RGB image
    image = Image.new('RGB', (width_px, height_px))

    # Convert the RGB565 data to RGB format and fill the image
    for i, rgb565 in enumerate(rgb565_data):
        x = i % width_px
        y = i // width_px
        if x < width_px and y < height_px:  # Check if the coordinates are within the image bounds
            rgb = rgb565_to_rgb888(rgb565)
            image.putpixel((x, y), rgb)

    # Save the image as a PNG file
    image.save(png_file_path)
    print(f"Image saved to {png_file_path}")
    # Print the header value as a hexadecimal string
    print(f"LVGL Header(hex): {header_int:#0{10}X}")

    # Return the header as an integer
    return header_int


def gen_rgb565_pattern(stripe_height_px=40, width_px=240, height_px=320):
    # Define the RGB565 colors
    RED_COLOR = 0xf800
    GREEN_COLOR = 0x07e0
    BLUE_COLOR = 0x001f

    # Calculate the total number of pixels in the display
    TOTAL_PIXELS = width_px * height_px
    # Initialize the display buffer with zeros
    lcd_buffer = [0] * TOTAL_PIXELS

    # Fill the display buffer with alternating color stripes
    for row in range(height_px // stripe_height_px):
        for col in range(width_px):
            # Calculate the color index for the pattern
            color_index = row % 3  # Cycle through red, green, blue
            if color_index == 0:
                color = RED_COLOR
            elif color_index == 1:
                color = GREEN_COLOR
            else:  # color_index == 2
                color = BLUE_COLOR
            for y in range(stripe_height_px):
                pixel_index = (row * stripe_height_px + y) * width_px + col
                if pixel_index < len(lcd_buffer):
                    lcd_buffer[pixel_index] = color

    return lcd_buffer

def convpng_to_rgb565(png_file_path, rgb_swap=False):
    try:
        with Image.open(png_file_path) as img:
            # Ensure image is in RGB format
            if img.mode != 'RGB':
                img = img.convert('RGB')

            # Get image size
            width_px, height_px = img.size

            # Create a list to store RGB565 data
            rgb565_data = []
            # Iterate over each pixel in the image
            for y in range(height_px):
                for x in range(width_px):
                    # Get the RGB values of the pixel
                    r, g, b = img.getpixel((x, y))

                    # Swap red and blue channels if rgb_swap is True
                    if rgb_swap:
                        r, b = b, r

                    # Convert the RGB values to RGB565 format
                    rgb565 = r8g8b8_to_rgb565(r, g, b)

                    # Append the 16-bit integer to the list
                    rgb565_data.append(rgb565)

            # Return RGB565 data and image width
            return rgb565_data, width_px, height_px

    except FileNotFoundError:
        print(f"The file {png_file_path} was not found.")
        return None, None, None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None, None, None

def append_lvgl_header(rgb565_data, width=240, height=320, format=4, output_file='output.bin'):
    # Ensure width, height, and format are non-negative integers
    assert isinstance(width, int) and width >= 0, "Width must be a non-negative integer"
    assert isinstance(height, int) and height >= 0, "Height must be a non-negative integer"
    assert isinstance(format, int) and format >= 0, "Format must be a non-negative integer"

    # Check if rgb565_data is a list of integers
    if not all(isinstance(x, int) for x in rgb565_data):
        raise ValueError("rgb565_data must be a list of integers.")

    # Create the header
    header_data = format
    header_data |= (width & 0x1FF) << 21  # Width is 9 bits
    header_data |= (height & 0x3FF) << 10  # Height is 10 bits

    # Convert the header to bytes
    header_bytes = struct.pack('<I', header_data)

    # Write the header and RGB565 data to the output file
    with open(output_file, 'wb') as outfile:
        outfile.write(header_bytes)
        for color in rgb565_data:
            outfile.write(struct.pack('<H', color))

    # Calculate the total byte length of the combined data
    total_bytes = 4 + len(rgb565_data) * 2 
    
    # Return the total byte length
    return total_bytes

# rgb565_data = gen_rgb565_pattern(stripe_height_px=40, width_px=80, height_px=80)
# rgb565_data, width, height = convpng_to_rgb565('test/1223.png')
# png_data = rgb565raw_show(combined_data, width, height)
# convbin_to_png('test/rgb.bin', 'test/rgb.png', width, height)

def append_rgb565_to_bin(rgb565_data, dest_bin_path):
    # Pack the RGB565 data into bytes
    rgb565_bytes = struct.pack(f'<{len(rgb565_data)}H', *rgb565_data)

    # Open the destination binary file in append mode
    with open(dest_bin_path, 'ab') as dest_file:
        # Get the size of the binary file before writing
        file_size = dest_file.tell()
        # Write the RGB565 data to the end of the destination file
        dest_file.write(rgb565_bytes)

    # Calculate the length of the appended data
    appended_length = len(rgb565_bytes)  # Number of bytes

    return appended_length, file_size

def convpng_from_dir(input_dir, output_dir):
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Get a list of all PNG files in the directory
    png_files = [f for f in os.listdir(input_dir) if f.lower().endswith('.png')]
    
    # Convert each PNG to RGB565 and save as a binary file
    for png_file in png_files:
        png_path = os.path.join(input_dir, png_file)
        rgb565_data, width, height = convpng_to_rgb565(png_path)
        
        # Check if the conversion was successful
        if rgb565_data is not None:
            # Create the output BIN file path
            bin_file_path = os.path.join(output_dir, os.path.splitext(png_file)[0] + '.bin')
            
            combined_length = append_lvgl_header(rgb565_data, width, height, 4, bin_file_path) 
            print(f"Converted {png_file} to {bin_file_path}")
        else:
            print(f"Failed to convert {png_file}")

# total_bytes = append_lvgl_header(rgb565_data, width, height, format, output_file)
# print(f"Total bytes written: {total_bytes}")
# Example usage:
# convpng_from_dir('.\images', '.\merge')

def convert_images_to_png_and_bin(input_dir, output_dir, max_width=240, max_height=320):
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Dictionary to keep track of the last counter for each size
    counters = {}
    
    # Iterate over all files in the input directory
    for filename in os.listdir(input_dir):
        # Check if the file is an image
        if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff')):
            file_path = os.path.join(input_dir, filename)
            try:
                # Open the image file
                with Image.open(file_path) as img:
                    # Get the dimensions of the image
                    width, height = img.size
                    
                    # Check if the image dimensions are within the specified range
                    if width > max_width or height > max_height:
                        print(f"Skipping {filename} as it exceeds the maximum dimensions ({max_width}x{max_height})")
                        continue
                    
                    # Generate a new PNG filename
                    name, ext = os.path.splitext(filename)

                    counter = counters.get((width, height), 0) + 1
                    new_filename_base = f"{width}_{height}_{counter:02d}_{name}"
                    new_png_filename = new_filename_base + '.png'
                    new_bin_filename = new_filename_base + '.bin'

                    counters[(width, height)] = counter
                    # Convert the image to PNG and save it
                    new_png_file_path = os.path.join(output_dir, new_png_filename)
                    img.save(new_png_file_path, 'PNG')

                    rgb565_data, width1, height1 = convpng_to_rgb565(new_png_file_path)
                    # Check if the conversion was successful
                    if rgb565_data is not None:
                        # Create the output BIN file path
                        bin_file_path = os.path.join(output_dir, new_bin_filename)
                        
                        combined_length = append_lvgl_header(rgb565_data,bin_file_path)
                        convbin_to_png(bin_file_path, new_png_file_path, width, height)

                    else:
                        print(f"Failed to convert {png_file}")

            except Exception as e:
                print(f"Error processing {filename}: {e}")

# Example usage:
# input_directory = './test/'  # Input directory path
# output_directory = './test/png'  # Output directory path
# convert_images_to_png_and_bin(input_directory, output_directory)

def convert_images_from_dir(directory_path, max_width=240, max_height=320):
    # 创建backup目录（如果不存在）
    backup_dir = os.path.join(directory_path, 'backup')
    if not os.path.exists(backup_dir):
        os.makedirs(backup_dir)

    # 遍历目录中的所有文件
    for filename in os.listdir(directory_path):
        file_path = os.path.join(directory_path, filename)
        # 如果文件是一个图片文件
        if os.path.isfile(file_path) and filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')):
            # 打开图片
            try:
                with Image.open(file_path) as img:
                    # 检查图片的尺寸是否超过了最大宽度和最大高度
                    if img.width > max_width or img.height > max_height:
                        print(f"Image {file_path} is too large, skipping conversion.")
                        # 将原图片移动到backup目录
                        backup_path = os.path.join(backup_dir, os.path.basename(file_path))
                        os.rename(file_path, backup_path)
                        print(f"Moved {file_path} to backup.")
                    else:
                        # 保存为PNG格式
                        png_path = os.path.splitext(file_path)[0] + '.png'
                        img.save(png_path, 'PNG')
                        print(f"Converted {file_path} to {png_path}.")
                        # 将原图片移动到backup目录
                        backup_path = os.path.join(backup_dir, os.path.basename(file_path))
                        os.rename(file_path, backup_path)
                        print(f"Moved original {file_path} to backup.")
            except Exception as e:
                print(f"Error converting {file_path} to PNG: {e}")

# 使用示例
# directory_path = './images/png'
# convert_images_from_dir(directory_path, max_width=240, max_height=320)

def mergebin_from_dir(input_dir, output_dir, output_prefix):
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)

    output_bin = os.path.join(output_dir, f'{output_prefix}.bin')
    output_h = os.path.join(output_dir, f'{output_prefix}.h')
    output_c = os.path.join(output_dir, f'{output_prefix}.c')

    # Get all .bin files in the input directory
    target_files = glob.glob(os.path.join(input_dir, '*.bin'))

    # Separate files into two lists: images and fonts
    image_files = []
    font_files = []
    for bin_file in target_files:
        basename = os.path.basename(bin_file)
        if re.match(r'^\d+_.*\.bin$', basename):
            image_files.append(bin_file)
        elif basename.lower().startswith('lv_font_'):
            font_files.append(bin_file)

    # Sort the image files by their numeric identifier
    image_files.sort(key=lambda x: int(re.search(r'^(\d+)_.*\.bin$', os.path.basename(x)).group(1)))

    # Sort the font files by their numeric identifier
    font_files.sort(key=lambda x: int(re.search(r'_(\d+)\.bin$', os.path.basename(x)).group(1)))

    # Convert each binary file and save as a binary file
    offset = 0
    with open(output_bin, 'wb') as out_bin_file, open(output_h, 'w') as out_h_file, open(output_c, 'w') as out_c_file:
        out_h_file.write("#ifndef EXT_FILES_H\n#define EXT_FILES_H\n\n")
        out_h_file.write("#include <stdint.h>\n\n")
        out_c_file.write("#include \"storage.h\"\n")
        out_h_file.write("#define FEATURE_SUBSYS_EXT_FILES_ENABLE\n\n")
        out_c_file.write(f"#include \"{os.path.basename(output_h)}\"\n\n")  # Include the generated .h file
        out_c_file.write("ext_bin_desc_t ext_bin_desc[] = {\n")

        # Process font files
        for bin_file in font_files:
            if "extern" in os.path.basename(bin_file):
                match = re.search(r'_(\d+)\.bin$', os.path.basename(bin_file))
                if match:
                    font_number = int(match.group(1))
                    font_define = os.path.basename(bin_file).replace('.bin', '').upper()
                    font_name = os.path.basename(bin_file)
                    font_file = "X:/" + font_name
                    out_h_file.write(f"#define {font_define}\t\"{font_file}\"\n")
                    length = os.path.getsize(bin_file)
                    with open(bin_file, 'rb') as infile:
                        data = infile.read()
                        out_bin_file.write(data)
                        lv_font_min = struct.unpack('<H', data[:2])[0]
                        lv_font_max = struct.unpack('<H', data[2:4])[0]
                        out_c_file.write(f"\t{{\"{font_name}\", 0x{offset:08X},{length},0x{lv_font_min:04X},0x{lv_font_max:04X},{0}}},\n")
                    offset += length
                    print(f"{font_name}, {length} Bytes")

        for bin_file in font_files:
            if "extern" in os.path.basename(bin_file):
                continue
            match = re.search(r'_(\d+)\.bin$', os.path.basename(bin_file))
            if match:
                font_number = int(match.group(1))
                font_define = os.path.basename(bin_file).replace('.bin', '').upper()
                font_name = os.path.basename(bin_file)
                font_file = "X:/" + font_name
                out_h_file.write(f"#define {font_define}\t\"{font_file}\"\n")
                length = os.path.getsize(bin_file)
                with open(bin_file, 'rb') as infile:
                    data = infile.read()
                    out_bin_file.write(data)
                    lv_font_min = struct.unpack('<H', data[:2])[0]
                    lv_font_max = struct.unpack('<H', data[2:4])[0]
                    out_c_file.write(f"\t{{\"{font_name}\", 0x{offset:08X},{length},0x{lv_font_min:04X},0x{lv_font_max:04X},{0}}},\n")
                offset += length
                print(f"{font_name}, {length} Bytes")
            else:
                print(f"Skipping file {bin_file} as it does not match the expected pattern.")

        # Process image files
        for bin_file in image_files:
            # Extract the numeric part of the image name
            match = re.search(r'^(\d+)_.*\.bin$', os.path.basename(bin_file))
            if match:
                image_number = match.group(1)
                # Calculate the length of the binary file
                length = os.path.getsize(bin_file)
                # Append the binary file to the output .bin file
                with open(bin_file, 'rb') as infile:
                    data = infile.read()
                    out_bin_file.write(data)
                    lv_header = data[:4]  # Assuming the header is the first 4 bytes
                    # Convert the lv_header from bytes to an integer
                    lv_header_int = struct.unpack('<I', lv_header)[0] 
                    lv_height = (lv_header_int >> 21) & 0x3FF 
                    lv_width = (lv_header_int >> 10) & 0x3FF  
                    lv_format = lv_header_int & 0xFF  
                    if data[:4] == b'\x89PNG':
                        image_name = os.path.basename(bin_file).replace('.bin', '.png')
                        image_file = "X:/" + image_name
                        with Image.open(bin_file) as img:
                            # Get the width and height of the image
                            png_width, png_height = img.size
                            # Write the define to the header file
                            out_h_file.write(f"#define LV_IMAGE_PNG_{image_number}    \"{image_file}\"\n")
                            # Write the entry to the C file
                            out_c_file.write(f"    {{\"{image_name}\", 0x{offset:08X}, {length},{png_width},{png_height},{0}}},\n")
                            print(f"Found {bin_file} as PNG file with dimensions {png_width}x{png_height}.")
                    else:
                        image_name = os.path.basename(bin_file)
                        image_file = "X:/" + image_name
                        out_h_file.write(f"#define LV_IMAGE_{image_number}    \"{image_file}\"\n")
                        out_c_file.write(f"    {{\"{image_name}\", 0x{offset:08X}, {length},{lv_width},{lv_height},{0}}},\n")

                offset += length
                # print(f"{image_name}, {length} Bytes")
            else:
                print(f"Skipping file {bin_file} as it does not match the expected pattern.")

        out_c_file.write("};\n\n")
        out_h_file.write(f"\n#define TOTAL_BIN_NUM\t\t{len(target_files)}\n")
        out_h_file.write(f"#define TOTAL_BIN_SIZE\t\t{offset}\n")
        out_h_file.write(typedef_struct)
        out_h_file.write("\n\n#endif // EXT_FILES_H\n")


def mergepng_from_dir(input_dir, output_dir, output_prefix):
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)

    output_bin = os.path.join(output_dir, f'{output_prefix}.bin')
    output_h = os.path.join(output_dir, f'{output_prefix}.h')
    output_c = os.path.join(output_dir, f'{output_prefix}.c')

    # Get a list of all PNG files in the directory
    png_files = glob.glob(os.path.join(input_dir, '*.png'))

    # Filter the PNG files to only include those that start with a digit
    png_files = [f for f in png_files if re.match(r'\d+.*\.png$', os.path.basename(f))]

    # Sort the PNG files by the numeric part of their names
    png_files.sort(key=lambda x: int(re.search(r'\d+', os.path.basename(x)).group()))

    # Create a list to hold the binary file paths
    bin_files = []
    # Convert each PNG to RGB565 and save as a binary file
    offset = 0
    with open(output_bin, 'wb') as out_bin_file, open(output_h, 'w') as out_h_file, open(output_c, 'w') as out_c_file:
        out_h_file.write("#ifndef EXT_FILES_H\n#define EXT_FILES_H\n\n")
        out_h_file.write("#include <stdint.h>\n\n")
        out_c_file.write("#include \"lv_port_fs.h\"\n")
        out_h_file.write("#define FEATURE_SUBSYS_EXT_FILES_ENABLE\n\n")
        out_c_file.write(f"#include \"{os.path.basename(output_h)}\"\n\n")  # Include the generated .h file
        out_c_file.write("ext_bin_desc_t ext_bin_desc[] = {\n")

        # Process PNG files
        for png_file in png_files:
            # Extract the numeric part of the image name
            image_number = re.search(r'\d+', os.path.basename(png_file)).group()
            # Write the definitions to the header file with the image name in double quotes
            image_name = os.path.basename(png_file).replace('.png', '.bin')
            image_file = "X:/" + image_name
            out_h_file.write(f"#define LV_IMAGE_{image_number}    \"{image_file}\"\n")

            # Convert the PNG to RGB565 and prepend the header
            rgb565_data, image_width, image_height = convpng_to_rgb565(png_file)
            size = append_lvgl_header(rgb565_data,output_bin)

            # Write the entry to the C source file
            out_c_file.write(f"    {{\"{image_name}\", 0x{offset:08X}, {size}, {image_width}, {image_height},{0}}},\n")

            # Update the offset for the next file
            offset += size

            # Print the file name and size
            print(f"{image_name}, {size} Bytes")

        out_c_file.write("};\n\n")
        out_h_file.write(f"#define TOTAL_BIN_NUM    {len(png_files)}\n")
        out_h_file.write(f"#define TOTAL_BIN_SIZE   {offset}\n\n")
        out_h_file.write("#endif // EXT_FILES_H\n")
        

def main():
    parser = argparse.ArgumentParser(description='Merge binary files from a directory.')
    parser.add_argument('input_dir', type=str, help='Input directory containing binary files.')
    parser.add_argument('output_dir', type=str, help='Output directory for the merged binary and header files.')
    parser.add_argument('output_prefix', type=str, help='Output file prefix for binary and header files.')
    args = parser.parse_args()
    # convpng_from_dir(args.input_dir, args.output_dir)
    mergebin_from_dir(args.input_dir, args.output_dir, args.output_prefix)

if __name__ == "__main__":
    main()
