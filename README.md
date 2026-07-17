# VS Code 平衡小车测试平台

这是一个基于 **STM32F1** 的平衡小车／电机控制测试项目。当前代码主要用于外设联调与底层控制模块验证，包含电机 PWM 控制、按键启停、电池电压监测，以及串口调试输出。

> 注意：项目目前仍处于功能验证和模块开发阶段，尚未实现完整的自平衡控制算法。更准确地说，它是一个可继续扩展的平衡小车控制测试平台。

## 已实现功能

- 通过 PWM 驱动电机
- 使用按键切换电机启用／禁用状态
- 采样电池电压，并以 LED 指示电量状态
- 通过 USART2 输出调试信息
- 基于 STM32 HAL 的外设初始化与驱动封装

### 电机控制

- 使用 TIM1 和 TIM4 输出 PWM 信号
- 通过 GPIO 控制左右电机的转向
- 通过 STBY 引脚控制电机驱动器的待机和工作状态

### 按键控制

- 读取 PA11 按键输入
- 对按键进行消抖处理，并切换电机状态
- 通过 USART2 输出当前状态，便于调试

### 电池监测

- 使用 ADC1 注入组转换采样电池电压
- 根据电压区间点亮不同的 LED 电量指示
- 电量过低时闪烁 LED 提示

### 串口调试

- 通过 USART2 输出调试信息
- 可用于观察按键事件、PWM 状态与电压变化

## 项目结构

```text
├─ Core/       STM32 HAL 初始化代码和主程序入口
├─ User/       电池、按键、PWM、串口等应用模块，以及基础测试代码
├─ MyLib/      延时、按键等通用工具模块
├─ Drivers/    STM32 HAL 与 CMSIS 驱动代码
└─ cmake/      CMake 与 ARM 交叉编译工具链配置
```

## 构建环境

- CMake 3.22 或更高版本
- Ninja
- ARM GNU Toolchain（`arm-none-eabi-gcc`）
- 可选：VS Code 与 CMake Tools 扩展

## 构建项目

项目已配置 CMake Preset。请在项目根目录执行：

```bash
cmake --preset Debug
cmake --build --preset Debug
```

也可以在 VS Code 的 CMake Tools 中直接选择 `Debug` 预设后进行配置和构建。

除 `Debug` 外，项目还提供以下构建预设：

- `RelWithDebInfo`
- `Release`
- `MinSizeRel`

例如，构建 Release 版本：

```bash
cmake --preset Release
cmake --build --preset Release
```

## 烧录固件

构建完成后会生成 ELF 和 BIN 文件，可使用 ST-Link、OpenOCD 或 STM32CubeProgrammer 烧录到开发板。

使用 OpenOCD 的示例：

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/vscode_balance_car.elf verify reset exit"
```

## 程序运行流程

上电后，固件会初始化各项外设，执行 PWM 测试程序，然后进入主循环，持续处理以下任务：

- 电池电压监测
- 按键扫描
- 电机控制状态更新

## 关键源码

- 主程序：`Core/Src/main.c`
- 电池处理：`User/Src/app_bat.c`
- 按键处理：`User/Src/app_button.c`
- PWM 电机控制：`User/Src/app_pwm.c`
- USART2 输出：`User/Src/app_usart2.c`
- PWM 测试：`User/test/pwm_test.c`

## 后续开发方向

若要将本项目发展为完整的平衡小车控制系统，建议逐步加入：

- IMU 传感器读取与姿态解算
- 角度环与速度环控制
- 电机编码器反馈
- PID 控制算法
- 倾倒、电压异常与输出限幅等保护机制
