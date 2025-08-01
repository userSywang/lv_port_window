# 搭建Windows系统CMake构建环境（免安装）

本文将介绍如何在Windows系统下搭建一个完整的CMake构建环境，包括安装必要的工具链、配置环境变量以及使用示例。

## 1 所需工具

要搭建一个完整的CMake构建环境，我们需要以下工具：

1. CMake - 跨平台的构建工具
2. MinGW-w64 GCC - Windows下的C/C++编译器
3. GNU Make - 构建工具

## 2 CMake构建环境搭建

### 2.1 工程目录结构

本项目采用以下目录结构组织文件：

```
winenv/
├── pkgs/               # 存放所需的工具包
│   ├── cmake-4.1.0-rc3-windows-x86_64.zip
│   ├── make-3.81-bin.zip
│   └── x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z
│   └── libs
│       └── SDL2-devel-2.32.4-mingw.zip
├── libs/               # 存放第三方库
│   └── SDL2-2.32.4     # SDL2库
├── prebuild/           # 工具包解压后的目录
│   ├── cmake-4.1.0-rc3-windows-x86_64/
│   ├── make-3.81-bin/
│   ├── gcc/
│   └── envfinished    # 环境安装完成的标记文件
├── scripts/           # 自动化脚本目录
│   ├── env.bat        # 环境变量配置脚本
│   └── winenvinstall.bat  # 工具包安装脚本
├── tools/             # 辅助工具目录
│   └── 7zr.exe       # 7z解压工具
├── example/           # 示例代码目录
│   └── helloworld/   # Hello World示例
│   └── sdl_demo/     # SDL示例
└── shell.bat         # 快速启动脚本
```

各目录说明：

1. **pkgs目录**：存放项目所需的工具包压缩文件
   - cmake-4.1.0-rc3-windows-x86_64.zip：CMake工具包
   - make-3.81-bin.zip：GNU Make工具包
   - x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z：MinGW-w64 GCC工具包

2. **prebuild目录**：工具包解压后的存放位置
   - cmake-4.1.0-rc3-windows-x86_64/：CMake工具解压目录
   - make-3.81-bin/：Make工具解压目录
   - gcc/win32/：GCC工具解压目录
   - envfinished：环境安装完成后的标记文件

3. **scripts目录**：存放项目的自动化脚本
   - env.bat：用于配置环境变量的脚本
   - winenvinstall.bat：用于自动安装工具包的脚本

4. **tools目录**：存放辅助工具
   - 7zr.exe：用于解压7z格式的工具包

5. **example目录**：存放示例代码
   - helloworld/：一个简单的Hello World示例项目

6. **shell.bat**：快速启动脚本，用于配置环境变量并打开命令行终端

项目源码：https://gitcode.com/win32grp/winenv.git

### 2.2 下载工具包

在本项目中，我们已经准备好了所需的工具包：

- CMake 4.1.0 RC3 (cmake-4.1.0-rc3-windows-x86_64.zip)
- GNU Make 3.81 (make-3.81-bin.zip)
- MinGW-w64 GCC 8.1.0 (x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z)

这些工具包都位于 `pkgs` 目录下。

### 2.3 解压工具包

本项目提供了自动化脚本 `scripts\winenvinstall.bat` 来处理工具包的解压和安装。脚本会：

1. 将CMake解压到 `prebuild\cmake-4.1.0-rc3-windows-x86_64` 目录
2. 将Make解压到 `prebuild\make-3.81-bin` 目录
3. 将GCC解压到 `prebuild\gcc\win32` 目录

winenvinstall.bat 脚本的内容如下：

```
@echo off
set ENV_ROOT_DIR=%~dp0..
set PREBUILD_DIR=%ENV_ROOT_DIR%\prebuild

REM 如果prebuild/cmake-4.1.0-rc3-windows-x86_64不存在
if not exist "%PREBUILD_DIR%\cmake-4.1.0-rc3-windows-x86_64" (
    echo "Install cmake ..."
    REM 将pkgs中的cmake-4.1.0-rc3-windows-x86_64.zip解压到prebuild目录
    powershell -Command "Expand-Archive -Path '%ENV_ROOT_DIR%\pkgs\cmake-4.1.0-rc3-windows-x86_64.zip' -DestinationPath %PREBUILD_DIR% -Force"
)

REM 如果prebuild/make-3.81-bin不存在
if not exist "%PREBUILD_DIR%\make-3.81-bin" (
    echo "Install make ..."
    REM 将pkgs中的make-3.81-bin.zip解压到prebuild目录
    powershell -Command "Expand-Archive -Path '%ENV_ROOT_DIR%\pkgs\make-3.81-bin.zip' -DestinationPath %PREBUILD_DIR% -Force"
)

REM 如果prebuild/gcc\win32\mingw64不存在
if not exist "%PREBUILD_DIR%\gcc\win32\mingw64" (
    echo "Install gcc ..."
    REM 如果gcc目录不存在
    if not exist "%PREBUILD_DIR%\gcc" (
        REM 创建gcc目录
        mkdir "%PREBUILD_DIR%\gcc"
    )
    REM 如果gcc/win32目录不存在
    if not exist "%PREBUILD_DIR%\gcc\win32" (
        REM 创建gcc/win32目录
        mkdir "%PREBUILD_DIR%\gcc\win32"
    )
    REM  使用tools/7zr.exe工具，从x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z解压gcc\win32"
    echo "Extracting gcc ..."
    "%ENV_ROOT_DIR%\tools\7zr.exe" x "%ENV_ROOT_DIR%\pkgs\x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z" -o"%PREBUILD_DIR%\gcc\win32" -y
)

REM 如果libs/SDL2-2.32.4不存在，则将pkgs/libs/SDL2-devel-2.32.4-mingw.zip解压到libs目录
if not exist "%ENV_ROOT_DIR%\libs\SDL2-2.32.4" (
    echo "Install libs ..."
    powershell -Command "Expand-Archive -Path '%ENV_ROOT_DIR%\pkgs\libs\SDL2-devel-2.32.4-mingw.zip' -DestinationPath '%ENV_ROOT_DIR%\libs' -Force"
)

REM 创建标记文件envfinished
echo "Setup finished. Create envfinished file."
type nul > %PREBUILD_DIR%\envfinished

pause
```

### 2.4 配置环境变量

本项目提供了 `scripts\env.bat` 脚本来自动配置所需的环境变量。这个脚本会：

1. 设置 `ENV_ROOT_DIR` 环境变量指向项目根目录
2. 将各个工具的bin目录添加到系统PATH中

你可以通过以下两种方式之一来初始化环境：

1. 双击根目录的 `shell.bat`
2. 直接运行 `scripts\env.bat`

env.bat 脚本的内容如下：
```
@echo off
set ENV_ROOT_DIR_TEMP=%~dp0..
set ENV_ROOT_DIR=%ENV_ROOT_DIR_TEMP:\scripts\..=%
echo ENV_ROOT_DIR is %ENV_ROOT_DIR%
set PATH=%ENV_ROOT_DIR%\prebuild\gcc\win32\mingw64\bin;%ENV_ROOT_DIR%\prebuild\cmake-4.1.0-rc3-windows-x86_64\bin;%ENV_ROOT_DIR%\prebuild\make-3.81-bin\bin
gcc --version
cmake --version
make --version

REM 设置cmake 第三方库全局搜索路径
set PATH=%ENV_ROOT_DIR%\libs;%PATH%

REM 检查是否存在envfinished文件
if exist "%ENV_ROOT_DIR%\prebuild\envfinished" (
    goto end
) else (
    echo Please execute scripts\winenvinstall.bat first.
)
:end

```

### 2.5 打开cmake编译终端

双击shell.bat，脚本会自动配置好环境变量，打开命令行cmake编译终端。
shelll.bat脚本内容如下：

```
@echo off

echo "Current cmake generator is %CMAKE_GENERATOR%"

REM 检查 CMAKE_GENERATOR 是否已定义且值正确
if defined CMAKE_GENERATOR (
    if not "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
        echo "set cmake generator to MinGW Makefiles"
        setx CMAKE_GENERATOR "MinGW Makefiles"
        echo Please restart the script
        pause 
        exit 0
    )
) else (
    echo "set cmake generator to MinGW Makefiles"
    setx CMAKE_GENERATOR "MinGW Makefiles"
    echo Please restart the script
    pause
    exit 0
)

cd /d "%~dp0"
cmd /k "scripts\env.bat"
```

## 3 使用示例

让我们通过项目中的HelloWorld示例来演示如何使用CMake进行项目构建。

### 3.1 准备工作
1. 下载winenv项目

```
git clone https://gitcode.com/win32grp/winenv.git

```

2. 进入winenv/scripts目录执行安装脚本winenvinstall.bat（下载winenv后只需要执行一次）

3. 进入winenv跟目录执行脚本shell.bat，脚本会自动配置好环境变量，打开命令行终端 

### 3.2 示例项目结构

```
example/helloworld/
├── CMakeLists.txt
└── main.c
```
main.c内容如下：
```
#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

### 3.3 构建步骤

1. 打开已配置好环境变量的终端（双击shell.bat）
2. 进入示例目录：
   ```bash
   cd example/helloworld
   ```
3. 创建并进入构建目录：
   ```bash
   mkdir build
   cd build
   ```
4. 生成构建文件：
   ```bash
   cmake ..
   ```
5. 执行构建：
   ```bash
   make
   ```

完成以上步骤后，你应该能在build目录下看到生成的可执行文件。

## 4 注意事项

1. 确保以管理员权限运行环境配置脚本
2. 如果遇到权限问题，检查是否以管理员身份运行终端
3. 确保系统已安装必要的运行时依赖

## 5 总结

通过本项目提供的工具包和脚本，你可以快速在Windows系统下搭建一个完整的CMake构建环境。这个环境包含了所有必要的工具，并通过自动化脚本简化了配置过程。你可以参考项目中的示例来开始使用这个构建环境进行开发。