# VS Code 平衡小车

基于 STM32F103（`STM32F103xB`）的两轮自平衡小车控制工程，使用 CMake 与 ARM GNU Toolchain 构建。项目采用 MPU6050 获取车身姿态、编码器测量轮速，并通过 TB6612 驱动左右直流电机。

> 当前控制算法仍在实车调参与验证阶段。请务必先将小车架空测试，确认电机、编码器和姿态角的正负方向，再进行地面平衡测试。

## 控制结构

控制由三层串级 PID 和独立的电机速度闭环组成：

```text
目标车速 v_ref（当前固定为 0 m/s）
        │
        ▼
速度环 PID ───────────────► 期望车身角度 theta_ref
        ▲
        │  速度反馈 x_dot（编码器 + 车身角速度补偿）

theta_ref ──► 角度环 PID ─► 期望角速度 theta_dot_ref
                                 │
                                 ▼
                         角速度环 PID
                                 │
                                 ▼
                          期望水平加速度 x_dot_dot_ref
                                 │
                                 ▼
                           积分得到目标轮速 omega_ref
                                 │
                                 ▼
                    左右电机速度闭环（编码器反馈 + PI）
                                 │
                                 ▼
                               PWM / TB6612
```

`omega_ref` 是目标轮速（rad/s），并非直接的 PWM 值。`App_Motor_SetSpeed_L/R()` 仅设置电机速度 PI 的设定值；电机模块以 1 ms 周期读取编码器速度，计算控制电压并转换为 PWM 占空比。

速度反馈使用下式补偿车身转动：

```c
omega  = (omega_l + omega_r) / 2;
omega2 = -theta_dot * (l + r) / r;
x_dot  = (omega - omega2) * r;
```

## 当前 PID 参数与执行周期

| 控制环 | 参数（Kp / Ki / Kd） | 输出限幅 | 周期 |
| --- | --- | --- | --- |
| 速度环 | 10 / 1 / 0 | ±0.4g m/s² | 5 ms |
| 车身角度环 | 4 / 0 / 0 | ±12.57 rad/s | 5 ms |
| 车身角速度环 | 10 / 10 / 0 | ±125.7 rad/s² | 5 ms |
| 左右电机速度环 | 0.55 / 7 / 0 | ±8.4 V | 1 ms |

这些参数依赖电机、轮径、重心、电池电压和传感器安装方向，不应直接视为通用参数。

## 主要功能

- MPU6050 姿态角与角速度读取
- 左右轮编码器测速（rad/s）
- 左右电机速度闭环与 PWM 驱动
- 平衡控制：速度、角度、角速度三级串级控制
- PA11 按键切换电机驱动使能；重新使能时复位控制器状态
- ADC 电池电压检测与低电量指示
- USART2 调试输出

## 目录结构

```text
Core/       CubeMX 生成的启动代码、外设初始化与 main.c
User/       应用模块：控制、电机、编码器、MPU6050、PWM、电池、按键等
MyLib/      通用库：PID、延时、定时任务、数学函数等
Drivers/    STM32 HAL 与 CMSIS
cmake/      工具链与 CubeMX CMake 配置
```

## 关键文件

- `User/Src/app_control.c`：三级平衡控制、速度反馈与 `omega_ref` 积分
- `User/Src/app_motor.c`：左右电机速度 PI 闭环与电压/PWM 换算
- `User/Src/app_encoder.c`：编码器方向判断和轮速估算
- `User/Src/app_mpu6050.c`：MPU6050 姿态与陀螺仪数据
- `User/Src/app_pwm.c`：TB6612 的方向与 PWM 输出
- `MyLib/Src/pid.c`：PID 实现
- `Core/Src/main.c`：初始化顺序和主循环调度

## 构建环境

- CMake 3.22 或更高版本
- Ninja
- ARM GNU Toolchain（`arm-none-eabi-gcc`）
- 可选：VS Code 与 CMake Tools / STM32Cube 扩展

## 构建

在项目根目录执行：

```bash
cmake --preset Debug
cmake --build --preset Debug
```

也可在 VS Code 中选择 `Debug` 预设构建。项目还提供 `Release`、`RelWithDebInfo` 与 `MinSizeRel` 预设；需要减小 Flash 占用时优先尝试 `Release` 或 `MinSizeRel`。

## 运行与调试注意事项

1. 第一次测试请将车架空，避免方向错误时冲出。
2. 确认正目标轮速时，电机实际转向与编码器读数符号一致。
3. 车身前倾时，轮子应向前运动以接住车身；后倾时相反。若方向相反，应检查 MPU6050、电机或编码器的符号约定。
4. `v_ref = 0` 表示最终期望整车速度为零，不表示轮子在扶正过程中不能转动。
5. 当前 `omega_ref` 由加速度积分得到。实车调试前建议为其增加合理限幅，防止异常姿态或错误反馈导致目标轮速持续累积。
6. 倒地、失控或改线时先通过按键禁用电机驱动，再重新使能以复位 PID 和 `omega_ref`。

## 烧录

构建后可使用 ST-Link、OpenOCD 或 STM32CubeProgrammer 烧录生成的 ELF 文件。例如：

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/vscode_balance_car.elf verify reset exit"
```
