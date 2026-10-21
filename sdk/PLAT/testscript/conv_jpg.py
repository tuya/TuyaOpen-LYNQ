import os
import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image

def convert_to_jpg():
    # 打开文件选择对话框，让用户选择一个图片文件
    file_path = filedialog.askopenfilename(
        title="Select an image file",
        filetypes=[("Image files", "*.png;*.bmp;*.gif;*.tiff;*.jpeg;*.jpg")]
    )
    
    if file_path:
        try:
            # 使用Pillow打开图片
            with Image.open(file_path) as img:
                # 获取文件名和目录
                file_dir, file_name = os.path.split(file_path)
                # 创建新的文件名，替换扩展名为.jpg
                new_file_name = os.path.splitext(file_name)[0] + '.jpg'
                new_file_path = os.path.join(file_dir, new_file_name)
                
                # 如果文件已经存在，则添加一个数字后缀
                counter = 1
                while os.path.exists(new_file_path):
                    new_file_name = os.path.splitext(file_name)[0] + f'_{counter}.jpg'
                    new_file_path = os.path.join(file_dir, new_file_name)
                    counter += 1
                
                # 转换图片格式为JPEG并保存
                img = img.convert('RGB')
                img.save(new_file_path, 'JPEG')
                
                messagebox.showinfo("Success", f"Image saved as {new_file_name}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to convert image: {e}")

# 创建主窗口
root = tk.Tk()
root.title("Image to JPG Converter")

# 设置窗口大小
root.geometry("300x100")

# 创建一个按钮，点击后执行转换操作
convert_button = tk.Button(root, text="Convert Image to JPG", command=convert_to_jpg)
convert_button.pack(pady=20)

# 运行主循环
root.mainloop()
