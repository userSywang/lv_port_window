# LVGL + ESP-Brookesia 在Windows下的编译和运行

## 1. 项目介绍

本项目是基于 LVGL（轻量级多功能图形库）和 ESP-Brookesia 的嵌入式模拟桌面应用开发框架，专为嵌入式设备构建丰富的图形界面而设计。通过在Windows环境下模拟嵌入式设备的图形界面，可以快速开发和测试嵌入式UI应用，无需实际硬件设备。

项目源码：[https://gitcode.com/aiprtem_lvgl/lv_port_window.git](https://gitcode.com/aiprtem_lvgl/lv_port_window.git)

### 1.1 主要特性

- 基于LVGL图形库，提供丰富的UI组件和动画效果
- 集成ESP-Brookesia框架，支持app式的应用管理方式
- 使用SDL2进行显示和输入设备模拟
- 支持多种开发环境：VSCode、ESP-IDF、Arduino
- 提供多种示例应用，包括简单控件演示和复杂应用界面
- 兼容Squareline Studio导出的UI设计代码

## 2. 环境搭建

在Windows系统下编译和运行LVGL + ESP-Brookesia项目，有两种方式：手动安装所需工具，或使用预配置的winenv开发环境。

### 2.1 手动安装编译环境

如果你希望手动安装所需工具，需要以下组件：

1. **CMake** (版本 3.12.4 或更高)
   - 下载地址：[https://cmake.org/download/](https://cmake.org/download/)
   - 安装时选择将CMake添加到系统PATH

2. **MinGW-w64 GCC** (支持C11和C++17标准)
   - 下载地址：[https://winlibs.com/](https://winlibs.com/)
   - 建议选择带有POSIX线程的版本
   - 安装后将bin目录添加到系统PATH

3. **GNU Make**
   - 下载地址：[https://ftp.gnu.org/gnu/make/](https://ftp.gnu.org/gnu/make/)
   - 选择与MinGW-w64版本匹配的版本
   - 安装后将bin目录添加到系统PATH

4. **SDL2库**
   - 下载地址：[https://github.com/libsdl-org/SDL/releases](https://github.com/libsdl-org/SDL/releases)
   - 选择MinGW开发库版本（SDL2-devel-x.x.x-mingw.zip）
   - 解压后将include和lib目录复制到MinGW安装目录下

### 2.2 使用winenv开发环境（推荐）

为了简化环境配置过程，推荐使用预配置的winenv开发环境：

1. **下载winenv**

   ```bash
   git clone https://gitcode.com/win32grp/winenv.git
   ```

2. **安装环境**

   进入winenv/scripts目录，运行winenvinstall.bat脚本。该脚本会自动解压以下工具：
   - CMake 4.1.0 RC3
   - GNU Make 3.81
   - MinGW-w64 GCC 8.1.0
   - SDL2 2.32.4

3. **启动开发环境**

   双击winenv根目录下的shell.bat脚本，将打开一个配置好环境变量的命令行终端。在这个终端中，你可以直接使用cmake、make、gcc等命令。

   winenv开发环境的使用方法请参考：[搭建Windows系统CMake构建环境.md](https://blog.csdn.net/prtem/article/details/149732744)

## 3. 源码获取

1. **克隆仓库及其子模块**

   ```bash
   git clone --recursive https://gitcode.com/aiprtem_lvgl/lv_port_window.git
   cd lv_port_window
   ```

   如果你已经克隆了仓库但没有包含子模块，可以使用以下命令获取子模块：

   ```bash
   git submodule update --init --recursive
   ```

## 4. 项目编译

### 4.1 使用命令行编译

1. **创建构建目录**

   ```bash
   mkdir build
   cd build
   ```

2. **配置CMake**

   ```bash
   cmake .. -G "MinGW Makefiles"
   ```

   注意：如果你使用winenv环境，已经自动设置了CMAKE_GENERATOR为"MinGW Makefiles"，可以直接使用：

   ```bash
   cmake ..
   ```

3. **编译项目**

   ```bash
   make
   ```

4. **安装到输出目录**

   ```bash
   make install
   ```

   默认情况下，编译好的可执行文件会安装到项目根目录下的out/lv_port_window/bin目录中。

## 5. 示例运行

项目提供了多个示例应用，位于examples目录下：

- **widget_demo**: 展示LVGL基础控件的使用
- **esp_brookesia_demo**: 演示ESP-Brookesia框架的基本功能
- **esp_brookesia_advanced**: 包含多个基于ESP-Brookesia的高级应用示例，如计算器、音乐播放器、游戏等

### 5.1 运行示例

编译完成后，可以在out/lv_port_window/bin目录下找到编译好的可执行文件：

```bash
# 在项目根目录下
cd out/lv_port_window/bin

# 运行基础控件演示
./widget_demo

# 运行ESP-Brookesia基本功能演示
./esp_brookesia_demo

# 运行ESP-Brookesia高级应用示例
./esp_brookesia_advanced
```

### 5.2 esp_brookesia_advanced示例

esp_brookesia_advanced是一个综合示例，展示了ESP-Brookesia框架的强大功能，包含以下应用：

1. **Calculator**：一个功能完整的计算器应用，支持基本运算和科学计算
2. **Draw**：绘图工具，支持触控或鼠标输入
3. **Game_2048**：经典的2048游戏，适配嵌入式设备的性能优化版本
4. **Music Player**：音乐播放器，支持本地音频文件播放和控制（只有界面，功能未实现）
5. **Video Player**：视频播放器，支持低分辨率视频的流畅播放（只有界面，功能未实现）

## 6. 项目结构解析

### 6.1 目录结构

- **/examples**：示例代码，展示LVGL组件的使用方法
  - **/esp_brookesia_advanced**：ESP-Brookesia高级功能示例
  - **/esp_brookesia_demo**：ESP-Brookesia基础演示示例
  - **/widget_demo**：LVGL组件演示示例
- **/lvgl**：LVGL核心库文件
- **/lv_drivers**：LVGL驱动程序库
- **/esp-brookesia**：ESP-Brookesia相关代码和配置
- **/CMakeLists.txt**：项目构建配置文件
- **/Makefile**：项目编译脚本
- **/lv_conf.h**：LVGL配置文件
- **/lv_drv_conf.h**：LVGL驱动程序配置文件

### 6.2 构建系统

项目使用CMake作为构建系统，主要的CMakeLists.txt文件包括：

- 根目录的CMakeLists.txt：定义整个项目的构建配置
- examples目录下的CMakeLists.txt：定义示例应用的构建配置
- 各个子目录下的CMakeLists.txt：定义具体模块的构建配置

## 7. ESP-Brookesia框架介绍

ESP-Brookesia是一个面向物联网设备的人机交互开发框架，由乐鑫（Espressif）开发，其主要特性包括：

### 7.1 核心功能

- **资源优化**：通过内存管理和算法优化，显著降低LVGL在嵌入式设备上的资源占用
- **性能提升**：针对嵌入式硬件特性（如低功耗CPU、有限内存）进行专项优化，确保图形界面流畅运行
- **硬件适配**：提供统一的硬件抽象层，支持快速移植到不同嵌入式平台
- **功耗管理**：集成智能功耗控制策略，延长设备续航时间

### 7.2 应用管理

- 采用C++开发，可在PC或ESP SoCs平台上编译
- 提供丰富的标准化系统UI，支持动态调整UI样式
- 采用app的应用管理方式，实现多个app的UI隔离与共存
- 应用UI兼容Squareline导出代码的开发方式

## 8. 自定义应用开发

### 8.1 创建新应用

1. 参考examples目录下的示例代码
2. 创建新的应用目录和CMakeLists.txt文件
3. 实现应用的主要功能和UI界面
4. 将新应用添加到主CMakeLists.txt中

### 8.2 应用集成

要将自定义应用集成到ESP-Brookesia框架中，可以参考esp_brookesia_demo/main.cpp中的代码：

```cpp
// 创建一个Phone对象
ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone();

// 选择并激活样式表
stylesheet = new ESP_Brookesia_PhoneStylesheet_t ESP_BROOKESIA_PHONE_1024_600_DARK_STYLESHEET();
phone->addStylesheet(stylesheet);
phone->activateStylesheet(stylesheet);

// 安装应用
YourCustomApp *app = new YourCustomApp();
phone->installApp(app);
```

## 9. 常见问题与解决方案

### 9.1 编译错误

**问题**：找不到SDL2库

**解决方案**：
- 确保SDL2库已正确安装
- 检查CMake配置中的SDL2路径是否正确
- 如果使用winenv环境，确保已运行winenvinstall.bat脚本

**问题**：编译时出现C++标准相关错误

**解决方案**：
- 确保使用的GCC编译器支持C++17标准
- 在CMake配置中添加`-std=c++17`标志

**问题**：ESP-Brookesia使用了指定初始化器（Designated Initializers）语法，导致在MINGW下编译失败

**解决方案**：
- 升级GCC版本到最新版
- 检查ESP-Brookesia代码，确保在使用指定初始化器语法的代码中不会跳过某些字段的初始化

### 9.2 运行错误

**问题**：运行时找不到SDL2.dll

**解决方案**：
- 将SDL2.dll复制到可执行文件所在目录
- 或将SDL2库的bin目录添加到系统PATH环境变量
- 或者静态链接SDL2库（推荐）

**问题**：窗口创建失败或黑屏

**解决方案**：
- 检查显示器分辨率设置
- 确保lv_conf.h中的分辨率配置正确
- 检查图形驱动初始化代码

## 10. 总结

这个开发环境为嵌入式UI开发提供了便捷的方式，让你可以在PC上快速开发和测试，然后再部署到实际的嵌入式设备上。