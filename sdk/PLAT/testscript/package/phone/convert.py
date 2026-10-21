import argparse
from PIL import Image

def rgb_to_rg565(r, g, b):
    # 将RGB数据转换为RG565格式
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def rg565_to_rgb(rg565):
    # 将RG565数据转换回RGB数据
    r = (rg565 >> 11) & 0x1F
    g = (rg565 >> 5) & 0x3F
    b = rg565 & 0x1F
    return (r << 3, g << 2, b << 3)

def convert_to_rg565(img):
    # 获取图片的像素数据
    pixels = img.getdata()
    
    # 将每个像素的RGB数据转换为RG565格式
    # rg565_pixels = [rgb_to_rg565(pixel[0], pixel[1], pixel[2]) for pixel in pixels]
    rg565_pixels = [rgb_to_rg565(pixel[2], pixel[1], pixel[0]) for pixel in pixels]
    # 创建一个新的图片对象
    new_img = Image.new('RGB', img.size)
    
    # 将RG565数据转换回RGB数据并设置新的像素数据
    new_pixels = [rg565_to_rgb(pixel) for pixel in rg565_pixels]
    new_img.putdata(new_pixels)
    
    return new_img

def convert_to_jpeg(input_image_path, output_image_path):
    # 打开输入图片
    with Image.open(input_image_path) as img:
        # 获取原始图片的尺寸
        original_size = img.size
        
        # 将图片转换为RGB模式（如果它不是RGB模式）
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # 将RGB数据转换为RG565格式
        img = convert_to_rg565(img)
        
        # 确保图片的尺寸与原始尺寸一致
        if img.size != original_size:
            img = img.resize(original_size)
        
        # 保存为JPEG格式
        img.save(output_image_path, 'JPEG')

if __name__ == "__main__":
    # 创建命令行参数解析器
    parser = argparse.ArgumentParser(description="Convert a 24-bit color image to a 16-bit color (RG565) image and save as JPEG.")
    
    # 添加输入图片路径参数
    parser.add_argument('input_image', type=str, help='Path to the input image file')
    
    # 添加输出目录参数
    parser.add_argument('output_directory', type=str, help='Path to the output directory')
    
    # 解析命令行参数
    args = parser.parse_args()
    
    # 获取输入图片路径
    input_image_path = args.input_image
    
    # 获取输出目录路径
    output_directory = args.output_directory
    
    # 构建输出图片路径
    output_image_path = f"{output_directory}/{input_image_path.split('/')[-1].split('.')[0]}.jpg"
    
    # 调用函数进行转换
    convert_to_jpeg(input_image_path, output_image_path)
    print(f"图片已保存为 {output_image_path}")