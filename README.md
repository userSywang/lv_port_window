# lv_port_window

本项目是基于 LVGL（轻量级多功能图形库）和 ESP-Brookesia 的嵌入式模拟桌面应用开发，专为嵌入式设备构建丰富的图形界面而设计。通过在Windows环境下模拟嵌入式设备的图形界面，可以快速开发和测试嵌入式UI应用，无需实际硬件设备。

## 项目概述

lv_port_window 提供了一个完整的开发环境，让开发者能够在PC上开发、测试和调试基于LVGL的嵌入式图形界面应用。项目集成了ESP-Brookesia框架，这是一个由乐鑫（Espressif）开发的人机交互开发框架，专为物联网设备设计，旨在简化UI设计和应用程序开发流程。

## 主要特性

- 基于LVGL图形库，提供丰富的UI组件和动画效果
- 集成ESP-Brookesia框架，支持app式的应用管理方式
- 使用SDL2进行显示和输入设备模拟
- 支持多种开发环境：VSCode、ESP-IDF、Arduino
- 提供多种示例应用，包括简单控件演示和复杂应用界面
- 兼容Squareline Studio导出的UI设计代码

## 安装与配置

### 前提条件

#### 手动安装编译环境

- CMake (版本 3.12.4 或更高)
- C/C++编译器 (支持C11和C++17标准)
- make (GNU make或任何其他兼容工具)
- SDL2库

#### 下载配置好的win开发环境

1.下载winenv

```bash
git clone https://gitcode.com/win32grp/winenv.git
```

2.进入文件夹winenv/scripts，运行winenvinstall.bat（该脚本只解压编译工具和库文件到prebuild和libs文件夹）

3.双击运行shell.bat进入mingww64的cmake shell环境。

winenv开发环境的使用方法请参考：https://blog.csdn.net/prtem/article/details/149732744

### 编译步骤

1. 克隆仓库及其子模块
   ```bash
   git clone --recursive https://gitcode.com/your-username/lv_port_window.git
   cd lv_port_window
   ```

2. 创建构建目录并配置CMake
   ```bash
   mkdir build && cd build
   cmake ..
   ```

3. 编译和安装
   ```bash
   make
   make install
   ```

   > 注意：`install`目标默认安装到`out`目录的`lv_port_window`子目录。如果需要调整安装目录，请使用`CMAKE_INSTALL_PREFIX`选项更改`CMakeLists.txt`文件。

## 使用方法

项目提供了多个示例应用，位于`examples`目录下：

- **widget_demo**: 展示LVGL基础控件的使用
- **esp_brookesia_demo**: 演示ESP-Brookesia框架的基本功能
- **esp_brookesia_advanced**: 包含多个基于ESP-Brookesia的高级应用示例，如计算器、音乐播放器、游戏等

运行示例应用：

```bash
# 在项目根目录下
cd out/lv_port_window/bin
./esp_brookesia_advanced
```

## 开发自定义应用

1. 参考`examples`目录下的示例代码
2. 创建新的应用目录和CMakeLists.txt文件
3. 实现应用的主要功能和UI界面
4. 将新应用添加到主CMakeLists.txt中

## ESP-Brookesia框架

ESP-Brookesia是一个面向物联网设备的人机交互开发框架，其主要特性包括：

- 采用C++开发，可在PC或ESP SoCs平台上编译
- 提供丰富的标准化系统UI，支持动态调整UI样式
- 采用app的应用管理方式，实现多个app的UI隔离与共存
- 应用UI兼容Squareline导出代码的开发方式

更多关于ESP-Brookesia的信息，请参阅[ESP-Brookesia文档](https://gitcode.com/aiprtem_lvgl/esp-brookesia)。

## 贡献

欢迎提交问题报告、功能请求和代码贡献。请遵循项目的代码风格和贡献指南。

## 许可证

本项目采用[MIT许可证](LICENSE)。