# vscode_balance_car

This project is a STM32F1-based test platform for a balance car / motor control application. The current code focuses on basic peripheral integration and low-level control modules, including motor PWM control, button-based enable/disable switching, battery voltage monitoring, and UART debug output.

> Note: Based on the current code, this project is still in the functional verification and module development stage. It does not yet implement a complete self-balancing control algorithm and is better described as a programmable test platform.

## Project Overview

The project uses a CMake + STM32CubeMX style structure and currently provides the following features:

- Motor driver control using PWM signals
- Button-based motor enable/disable switching
- Battery voltage sampling and status LED indication
- USART2 debug output
- HAL-based peripheral initialization and driver abstraction

## Current Implemented Features

### 1. Motor Control
- PWM output through TIM1 and TIM4
- Direction control for left and right motors using GPIO pins
- STBY pin control to put the motor driver into sleep or active mode

### 2. Button Control
- Reads the PA11 button input
- Debounces the button input and toggles the motor state
- Prints the current state through USART2

### 3. Battery Monitoring
- Uses ADC1 injected conversion to sample battery voltage
- Drives different LED levels according to battery voltage range
- Blinks LEDs when the battery is low

### 4. Debug Output
- Sends debug information through USART2
- Useful for observing button actions, PWM state, and voltage changes

## Project Structure

- Core/
  - STM32 HAL initialization code and the main program entry
- User/
  - Application modules such as battery, button, PWM, and USART handling
  - Test sources for basic functional validation
- MyLib/
  - Utility modules such as delay helpers
- Drivers/
  - STM32 HAL and CMSIS driver code
- cmake/
  - CMake and cross-compilation toolchain configuration

## Build Instructions

The project is already configured with CMake presets. Build it with:

```bash
cmake --preset Debug
cmake --build --preset Debug
```

If you are using VS Code with CMake Tools, you can also select the Debug preset directly.

## Flashing Instructions

After building, the workspace will generate ELF/BIN outputs. You can flash them using ST-Link, OpenOCD, or STM32CubeProgrammer.

Example with OpenOCD:

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/vscode_balance_car.elf verify reset exit"
```

## Runtime Behavior

On startup, the firmware initializes the peripherals, runs the PWM test routine, and then enters the main loop to continuously handle:

- Battery monitoring
- Button scanning
- Motor control state updates

## Key Source Files

- Main program: Core/Src/main.c
- Battery handling: User/Src/app_bat.c
- Button handling: User/Src/app_button.c
- PWM motor control: User/Src/app_pwm.c
- USART output: User/Src/app_usart2.c
- PWM test routine: User/test/pwm_test.c

## Future Development Suggestions

To evolve this into a real balance car control system, the next steps would typically include:

- IMU sensor reading and attitude estimation
- Angle loop and speed loop control
- Motor encoder feedback
- PID control implementation
- Protection and saturation logic for unstable operation
