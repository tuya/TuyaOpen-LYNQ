import os
import glob
import array
import struct
import argparse
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
        out_h_file.write("#ifndef _EXT_BIN_H_\n#define _EXT_BIN_H_\n\n")
        out_h_file.write("#include <stdint.h>\n\n")
        out_c_file.write(f"#include \"{os.path.basename(output_h)}\"\n\n")  # Include the generated .h file
        out_c_file.write("ext_bin_desc_t ext_bin_desc[] = {\n")

        # Process font files
        for bin_file in font_files:
            match = re.search(r'_(\d+)\.bin$', os.path.basename(bin_file))
            if match:
                font_number = int(match.group(1))
                font_define = os.path.basename(bin_file).replace('.bin', '').upper()
                font_name = os.path.basename(bin_file)
                font_file = "X:/" + font_name
                out_h_file.write(f"#define {font_define}\t\"{font_file}\"\n")
                out_h_file.write(f"#define {font_define}_LNA\t0x{offset:08X}\n")
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
                            out_h_file.write(f"#define EXT_BIN_PNG_{image_number}    \"{image_file}\"\n")
                            # Write the entry to the C file
                            out_c_file.write(f"    {{\"{image_name}\", 0x{offset:08X}, {length},{png_width},{png_height},{0}}},\n")
                            print(f"Found {bin_file} as PNG file with dimensions {png_width}x{png_height}.")
                    else:
                        image_name = os.path.basename(bin_file)
                        image_file = "X:/" + image_name
                        out_h_file.write(f"#define EXT_BIN_{image_number}    \"{image_file}\"\n")
                        out_c_file.write(f"    {{\"{image_name}\", 0x{offset:08X}, {length},{lv_width},{lv_height},{0}}},\n")

                offset += length
                # print(f"{image_name}, {length} Bytes")
            else:
                print(f"Skipping file {bin_file} as it does not match the expected pattern.")

        out_c_file.write("};\n")
        out_h_file.write(f"\n#define TOTAL_BIN_NUM\t{len(target_files)}\n")
        out_h_file.write(f"#define TOTAL_BIN_SIZE\t{offset}\n")
        out_h_file.write(typedef_struct)
        out_h_file.write("\n#endif // _EXT_BIN_H_\n")

def main():
    parser = argparse.ArgumentParser(description='Merge binary files from a directory.')
    parser.add_argument('input_dir', type=str, help='Input directory containing binary files.')
    parser.add_argument('output_dir', type=str, help='Output directory for the merged binary and header files.')
    parser.add_argument('output_prefix', type=str, help='Output file prefix for binary and header files.')
    args = parser.parse_args()
    mergebin_from_dir(args.input_dir, args.output_dir, args.output_prefix)

if __name__ == "__main__":
    main()