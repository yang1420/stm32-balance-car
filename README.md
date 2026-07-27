# VS Code 平衡小车

基于 STM32F103（`STM32F103xB`）的两轮自平衡小车控制工程，使用 CMake 与 ARM GNU Toolchain 构建。项目使用 MPU6050 获取姿态、编码器测量轮速，通过 TB6612 驱动左右直流电机，并支持 USART3 蓝牙遥控。

> 控制算法仍在实车调参与验证阶段。首次测试必须架空小车；确认电机、编码器和姿态角的正负方向正确后，再进行地面平衡测试。

## 控制结构

系统由三级串级平衡控制、电机速度闭环和独立的偏航转向环组成：

```text
目标线速度 v_ref（蓝牙命令）
        │
        ▼
速度环 PID ───────────────────► 期望车身角度 theta_ref
        ▲
        │  x_dot（编码器 + 车身角速度补偿）

theta_ref ──► 角度环 PID ─────► theta_dot_ref
                                      │
                                      ▼
                              角速度环 PID
                                      │
                                      ▼
                                水平加速度 x_dot_dot_ref
                                      │
                                      ▼
                              积分得到共同轮速 omega_ref

目标偏航角速度 ──► 转向 PID ──► 差动轮速 omega_diff
                                      │
                 左轮目标：omega_ref + omega_diff
                 右轮目标：omega_ref - omega_diff
                                      │
                                      ▼
                左右电机速度 PI（编码器反馈，1 ms）
                                      │
                                      ▼
                                  PWM / TB6612
```

`omega_ref` 和 `omega_diff` 都是轮速目标（rad/s），不是 PWM。`App_Motor_SetSpeed_L/R()` 仅设置左右电机速度 PI 的设定值；电机模块读取编码器实际速度，计算控制电压，再转换为 PWM 占空比。

速度反馈对车身转动进行补偿：

```c
omega  = (omega_l + omega_r) / 2;
omega2 = -theta_dot * (l + r) / r;
x_dot  = (omega - omega2) * r;
```

转向 PID 使用 MPU6050 的 Z 轴角速度 `gz` 作为反馈。转向项对左右轮一加一减：理想情况下，它在左右轮平均值中抵消，不直接改变前后速度；实际仍会受到电机限幅、地面摩擦和轮胎打滑影响。

## 当前 PID 参数与执行周期

| 控制环 | 参数（Kp / Ki / Kd） | 输出限幅 | 周期 |
| --- | --- | --- | --- |
| 速度环 | 10 / 1 / 0 | ±0.4g m/s² | 5 ms |
| 车身角度环 | 4 / 0 / 0 | ±12.57 rad/s | 5 ms |
| 车身角速度环 | 20 / 20 / 0 | ±125.7 rad/s² | 5 ms |
| 偏航转向环 | 1 / 0 / 0 | ±15 rad/s | 5 ms |
| 左右电机速度环 | 0.55 / 7 / 0 | ±8.4 V | 1 ms |

`omega_ref` 由期望水平加速度积分得到，当前限制为 `±20 rad/s`。这些参数与电机、电池、轮径、重心和传感器安装方向强相关，不应直接视为通用参数。

## 蓝牙遥控

蓝牙模块接在 USART3，配置为 `9600-8-N-1`：

```text
PB10 → USART3_TX
PB11 → USART3_RX
```

接收使用 HAL 单字节中断。收到换行符 `\n` 后，将一整行 ASCII 命令交给主循环解析。命令格式：

```text
move <turn> <speed>\n
```

`turn` 与 `speed` 是整数，建议范围 `-100` 到 `100`。当前换算关系为：

```text
v_ref            = -speed × 0.007 m/s
yaw_rate_ref     = -turn  × 0.15 rad/s
```

例如：

```text
move 0 50
```

该命令不请求转向，并设置线速度目标为 `-0.35 m/s`。正负号受电机、IMU 和编码器的物理安装方向影响；若实际前后或左右方向与预期相反，应在命令换算处调整符号。

## 主要功能

- MPU6050 姿态角、三轴角速度读取
- 左右轮编码器测速（rad/s）
- 左右电机速度 PI 闭环和 PWM 驱动
- 速度、角度、角速度三级串级平衡控制
- 基于偏航角速度反馈的差动转向控制
- USART3 蓝牙 `move` 前进、后退和转向命令
- PA11 按键切换电机驱动使能；重新使能时复位平衡控制状态
- ADC 电池电压检测与低电量指示
- USART2 调试输出

## 目录结构

```text
Core/       CubeMX 生成的启动代码、外设初始化与 main.c
User/       控制、电机、编码器、MPU6050、PWM、蓝牙遥控、电池、按键等应用模块
MyLib/      PID、延时、周期任务、数学函数等通用库
Drivers/    STM32 HAL 与 CMSIS
cmake/      工具链与 CubeMX CMake 配置
```

## 关键文件

- `User/Src/app_control.c`：三级平衡控制、转向 PID、速度反馈、轮速积分和混控
- `User/Src/app_motor.c`：左右电机速度 PI 闭环与电压/PWM 换算
- `User/Src/app_encoder.c`：编码器方向判断和轮速估算
- `User/Src/app_mpu6050.c`：MPU6050 姿态与陀螺仪数据
- `User/Src/app_remote_control.c`：USART3 中断接收和 `move` 命令解析
- `User/Src/app_pwm.c`：TB6612 的方向与 PWM 输出
- `MyLib/Src/pid.c`：PID 实现
- `Core/Src/main.c`：初始化顺序和主循环调度

## 构建

依赖：CMake 3.22+、Ninja 和 ARM GNU Toolchain（`arm-none-eabi-gcc`）。在项目根目录执行：

```bash
cmake --preset Debug
cmake --build --preset Debug
```

也可在 VS Code 中选择 `Debug` 预设构建。项目还提供 `Release`、`RelWithDebInfo` 和 `MinSizeRel`；需要减小 Flash 占用时优先尝试 `Release` 或 `MinSizeRel`。

## 运行与调试注意事项

1. 第一次测试请架空小车，避免方向错误时冲出。
2. 正目标轮速时，电机实际转向与编码器读数符号必须一致。
3. 车身前倾时，轮子应向前运动以接住车身；后倾时相反。若方向相反，应检查 MPU6050、电机或编码器的符号约定。
4. `v_ref = 0` 表示最终期望整车速度为零，不表示轮子在扶正过程中不能转动。
5. 先用较小的 `move` 命令幅值测试；大速度或大转向命令会挤占平衡控制所需的电机能力。
6. 倒地、失控或改线时先通过按键禁用电机驱动，再重新使能以清除 PID 状态和 `omega_ref`。

## 烧录

构建后可使用 ST-Link、OpenOCD 或 STM32CubeProgrammer 烧录 ELF 文件。例如：

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/vscode_balance_car.elf verify reset exit"
```
