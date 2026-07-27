# VS Code Balance Car

This is a two-wheel self-balancing car project for STM32F103 (`STM32F103xB`). It is built with CMake and the ARM GNU Toolchain. The car uses an MPU6050 for attitude sensing, wheel encoders for speed feedback, a TB6612 motor driver, and USART3 Bluetooth remote control.

> The control algorithm is still being tuned and validated on real hardware. Always test with the car lifted off the ground first. Verify the signs of motor direction, encoder speed, and attitude angle before testing on the floor.

## Control Architecture

The system consists of three cascaded balance-control loops, independent motor speed loops, and a yaw-rate turning loop:

```text
Target linear speed v_ref (Bluetooth command)
        │
        ▼
Speed PID ─────────────────────► Desired body angle theta_ref
        ▲
        │  x_dot (encoder speed + body angular-rate compensation)

theta_ref ──► Angle PID ───────► theta_dot_ref
                                      │
                                      ▼
                              Angular-rate PID
                                      │
                                      ▼
                              Desired acceleration x_dot_dot_ref
                                      │
                                      ▼
                         Integrated common wheel speed omega_ref

Target yaw rate ──► Turn PID ──► Differential speed omega_diff
                                      │
                 Left target:  omega_ref + omega_diff
                 Right target: omega_ref - omega_diff
                                      │
                                      ▼
                  Left/right motor speed PI loops (encoder feedback, 1 ms)
                                      │
                                      ▼
                                  PWM / TB6612
```

`omega_ref` and `omega_diff` are wheel-speed references in rad/s, not PWM values. `App_Motor_SetSpeed_L/R()` only updates the setpoint of each motor speed PI controller. The motor module reads the actual encoder speed, calculates a control voltage, and converts it to PWM duty cycle.

The velocity feedback compensates for body rotation:

```c
omega  = (omega_l + omega_r) / 2;
omega2 = -theta_dot * (l + r) / r;
x_dot  = (omega - omega2) * r;
```

The turn PID uses MPU6050 Z-axis angular velocity `gz` as feedback. Its output is added to one wheel target and subtracted from the other. Ideally, the turning term cancels out in the average wheel speed and does not directly change forward speed; in practice, motor saturation, tire friction, and wheel slip can couple the two controls.

## Current PID Parameters and Periods

| Control loop | Parameters (Kp / Ki / Kd) | Output limit | Period |
| --- | --- | --- | --- |
| Speed loop | 10 / 1 / 0 | ±0.4g m/s² | 5 ms |
| Body-angle loop | 4 / 0 / 0 | ±12.57 rad/s | 5 ms |
| Body angular-rate loop | 20 / 20 / 0 | ±125.7 rad/s² | 5 ms |
| Yaw-rate turn loop | 1 / 0 / 0 | ±15 rad/s | 5 ms |
| Left/right motor speed loops | 0.55 / 7 / 0 | ±8.4 V | 1 ms |

`omega_ref` is obtained by integrating the requested horizontal acceleration and is currently limited to `±20 rad/s`. These parameters depend on the motors, battery, wheel radius, center of mass, and sensor mounting direction; they are not universal tuning values.

## Bluetooth Remote Control

The Bluetooth module is connected to USART3 and configured as `9600-8-N-1`:

```text
PB10 → USART3_TX
PB11 → USART3_RX
```

Reception uses HAL single-byte interrupts. Once a newline character (`\n`) is received, the complete ASCII line is passed to the main loop for parsing. Command format:

```text
move <turn> <speed>\n
```

`turn` and `speed` are integers; the recommended range is `-100` to `100`. The current conversion is:

```text
v_ref        = -speed × 0.007 m/s
yaw_rate_ref = -turn  × 0.15 rad/s
```

For example:

```text
move 0 50
```

This command requests no turning and sets the linear-speed reference to `-0.35 m/s`. The physical meaning of positive/negative direction depends on the motor, IMU, and encoder installation. If forward/backward or left/right is reversed, adjust the sign in the command conversion.

## Features

- MPU6050 body-angle and three-axis angular-rate readings
- Left and right wheel encoder speed measurement in rad/s
- Left and right motor speed PI control with PWM drive
- Cascaded speed, angle, and angular-rate balance control
- Differential turning control using yaw-rate feedback
- USART3 Bluetooth `move` commands for forward, reverse, and turning motion
- PA11 button for motor enable/disable; re-enabling resets the balance-control state
- ADC battery-voltage monitoring and low-battery indication
- USART2 debug output

## Directory Layout

```text
Core/       CubeMX-generated startup code, peripheral initialization, and main.c
User/       Application modules: control, motor, encoder, MPU6050, PWM, Bluetooth,
             battery, button, and related code
MyLib/      Common libraries: PID, delay, periodic tasks, and math functions
Drivers/    STM32 HAL and CMSIS
cmake/      Toolchain and CubeMX CMake configuration
```

## Important Source Files

- `User/Src/app_control.c`: cascaded balance control, turn PID, velocity feedback, wheel-speed integration, and motor mixing
- `User/Src/app_motor.c`: motor speed PI loops and voltage/PWM conversion
- `User/Src/app_encoder.c`: encoder direction handling and wheel-speed estimation
- `User/Src/app_mpu6050.c`: MPU6050 attitude and gyroscope data
- `User/Src/app_remote_control.c`: USART3 interrupt reception and `move` command parsing
- `User/Src/app_pwm.c`: TB6612 direction and PWM output
- `MyLib/Src/pid.c`: PID implementation
- `Core/Src/main.c`: initialization order and main-loop scheduling

## Build

Requirements: CMake 3.22 or newer, Ninja, and the ARM GNU Toolchain (`arm-none-eabi-gcc`). Run the following commands from the project root:

```bash
cmake --preset Debug
cmake --build --preset Debug
```

You can also build with the `Debug` preset in VS Code. The project also provides `Release`, `RelWithDebInfo`, and `MinSizeRel` presets. Use `Release` or `MinSizeRel` when reducing Flash usage is important.

## Running and Debugging Notes

1. Keep the car lifted for the first test to prevent uncontrolled movement caused by an incorrect sign.
2. For a positive wheel-speed reference, the physical motor direction and encoder speed sign must agree.
3. When the body tilts forward, the wheels should move forward to catch the body; when it tilts backward, they should move backward. If not, check the sign conventions of the MPU6050, motor, and encoder.
4. `v_ref = 0` means the desired final vehicle speed is zero. It does not mean that the wheels must remain still while correcting body tilt.
5. Start with small `move` command values. Large speed or turn commands can consume motor authority needed for balancing.
6. If the car falls, becomes unstable, or is being rewired, disable the motor driver with the button first. Re-enable it afterwards to clear PID state and `omega_ref`.

## Flashing

After building, flash the generated ELF file with ST-Link, OpenOCD, or STM32CubeProgrammer. For example:

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/vscode_balance_car.elf verify reset exit"
```
