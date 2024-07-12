import tkinter as tk
from tkinter import filedialog  #文件资源管理器操作相关
from tkinter import messagebox  #操作系统信息显示相关
from tkinter import ttk
import subprocess  # 系统命令模块
import serial
from serial.tools import list_ports

selected_port=None
new_content=None

def update_com_port(event=None): #没有GUI事件触发的时候也能调用此函数
    global selected_port #调用全局变量的提前声明
    selected_port = com_port_var.get() # 用全局变量存储选择的com口
    messagebox.showinfo("注意",f"已选择{selected_port}")

def list_available_ports():
    ports = [port.device for port in serial.tools.list_ports.comports()] #便利迭代器并且只取port的device属性
    return ports

def templeprogram(event=None):  # event=None表示既可以被事件对象触发也可以被直接的函数调用所触发
    global  new_content
    ssid = ssid_entry.get()
    password = password_entry.get()
    table_number = table_number_entry.get()
    with open('Arduino15/sketches/sketches.ino', 'r', encoding='utf-8') as file:  # 使用上下文管理机制进行有效IO管理
        content = file.read()                                                     # 将读取到的新的连续字符串用content存储
    # 替换或插入SSID、密码和桌号
    new_content = content.replace("const char *ssid = \"\";", f"const char *ssid = \"{ssid}\";")  # 暂存修改结果到运行内存
    new_content = new_content.replace("const char *password = \"\";",
                                      f"const char *password = \"{password}\";")  # 暂存修改结果到运行内存
    new_content = new_content.replace("const char *mahjiongcode=\"\";",
                                      f"const char *mahjiongcode=\"{table_number}\";")  # 暂存修改结果到运行内存
    try:
        with open('Arduino15/sketches/sketches_amend/sketches_amend.ino', 'w', encoding='utf-8') as file:
            file.write(new_content)
    except:
        messagebox.showerror("错误","修改失败")
    else:
        messagebox.showinfo("提示","修改成功")

    # messagebox.showerror("k",new_content) #打印装填信息

    """执行修改源码命令"""

def compile_():
    """烧录设备"""
    # 设置Arduino CLI的路径（如果已加入PATH环境变量，这一步可以省略）
    arduino_cli_path = "arduino-cli.exe"  # 或者是绝对路径，如 "/usr/local/bin/arduino-cli"，调用命令行时候不加.exe后缀

    if not (password_entry and table_number_entry and ssid_entry):
        messagebox.showerror("错误","请确保填写完相应信息")
        return 0
    compile_command = [arduino_cli_path, "compile", "--fqbn", "esp8266:esp8266:nodemcuv2", "Arduino15/sketches/sketches_amend", "--config-file", "arduino-cli.yaml"]
    try:
        subprocess.run(compile_command, check=True)
        messagebox.showinfo("提醒","编译成功")
    except subprocess.CalledProcessError as e:
        print(f"Compilation error: {e}")

#https://arduino.github.io/arduino-cli/0.20/commands/arduino-cli_compile/#options编译命令解释地址


def flash_device():
    input_dir="Arduino15/sketches/sketches_amend/build/esp8266.esp8266.nodemcuv2"
    upload_command = ["arduino-cli.exe", "upload","--fqbn","esp8266:esp8266:nodemcuv2","--input-dir",input_dir,"-p",selected_port,"--config-file","arduino-cli.yaml"]
    try:
         subprocess.run(upload_command, check=True) #执行终端命令

    except subprocess.CalledProcessError as e:
         messagebox.showerror("错误",f"{e}")




# 创建主窗口,窗口内的函数皆为异步函数
app = tk.Tk()
app.title("ESP32/ESP8266烧录工具")  #窗口标题

# 在GUI中添加输入框和按钮
ssid_label = tk.Label(app, text="WiFi名字(不能有中文):")
ssid_label.pack()
ssid_entry = tk.Entry(app, width=50)
ssid_entry.pack()
# 添加WiFi密码
password_label = tk.Label(app, text="WiFi密码:")
password_label.pack()
password_entry = tk.Entry(app, width=50, show="*")  # 显示密码为星号
password_entry.pack()
# 添加桌号
table_number_label = tk.Label(app, text="新增桌号:")
table_number_label.pack()
table_number_entry = tk.Entry(app, width=50)
table_number_entry.pack()




# 变量用于存储选择的串口
com_port_var = tk.StringVar()

# 获取所有可用的串口
available_ports = list_available_ports() # 非空则视为True的bool值返回
if available_ports:
    # 设置默认值为第一个找到的串口
    default_port = available_ports[0]
else:
    default_port = "No Ports Available"

# 创建下拉列表
com_port_dropdown = ttk.Combobox(app, textvariable=com_port_var, values=available_ports) #调用Combobox方法，指定父窗口，以及下拉列表要显示选项变量,以及传出的选中变量
com_port_dropdown.set(default_port)  # 设置默认显示的值
com_port_dropdown.bind("<<ComboboxSelected>>", update_com_port)  # 绑定选择事件处理函数
com_port_dropdown.pack(padx=10, pady=10)






# 修改程序
flash_button = tk.Button(app, text="修改程序", command=templeprogram)
flash_button.pack()




# 编译按钮
flash_button = tk.Button(app, text="编译", command=compile_) #点击按钮向flash_device函数传递事件对象
flash_button.pack()                                             #这个command参数与bind方法不一样，执行时候不会朝被调用的函数传递对象

# 烧录按钮
flash_button = tk.Button(app, text="烧录", command=flash_device)
flash_button.pack()




# 运行应用程序
app.mainloop()

#已实现将主要信息装如内存文件内 Y
#实现选择串口  Y
#实现arduino cli编译和存储编译结果 Y
#纠正arduino cli的command传递参数 Y
#实现上传开发板Y
#实现打印上传进度条N
#程序体积精简N


