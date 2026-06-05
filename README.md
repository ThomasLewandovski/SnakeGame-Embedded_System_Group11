# Embedded Tilt-Controlled Snake Game

EBU5477 Embedded Systems Design Mini-Project: Snake game on STM32F401RE with OLED display, K1-K4 buttons, optional MPU6500 tilt input, DS1820 temperature sensing, and 16x2 LCD status output.

## Current Project Status

The current formal Keil project is:

```text
Miniproj/SnakeGame.uvprojx
```

The formal build target is:

```text
SnakeGame_Release
```

The normal build output name is:

```text
SnakeGame.axf
SnakeGame.hex
```

The old demo project naming has been removed from the active project files. Keil may still generate local user layout files such as `*.uvguix.*`; those are not source files and should not be submitted.

## Hardware Mapping

```text
OLED / MPU6500 I2C:
SCL -> PB8
SDA -> PB9

OLED buttons:
K1 -> PC8
K2 -> PC6
K3 -> PC5
K4 -> PC9

DS1820 / DS18B20:
DQ -> PA1

16x2 LCD:
RS -> PC7
E  -> PA9
D7 -> PA8
D6 -> PB10
D5 -> PB4
D4 -> PB5
```

## Entry Points

The default formal entry file is:

```text
Miniproj/src/main.c
```

It calls:

```c
App_Main();
```

This keeps tilt control disabled by default and runs the stable button-controlled game.

There is also a parallel tilt-test entry file:

```text
Miniproj/src/main_tilt.c
```

It calls:

```c
App_MainWithTilt(1U);
```

In the Keil project, `main_tilt.c` is present but excluded from build by default. To test tilt control, disable `main.c` from build and enable `main_tilt.c`. Do not build both at the same time because both files contain `main()`.

Keil switching steps:

1. Open `Miniproj/SnakeGame.uvprojx`.
2. In the left `Project` file tree, expand target `SnakeGame_Release`.
3. Expand group `Entry`.
4. Right-click `main.c`, open `Options for File 'main.c'`, then tick `Exclude from Build`.
5. Right-click `main_tilt.c`, open `Options for File 'main_tilt.c'`, then untick `Exclude from Build`.
6. Build and Download again.
7. For normal button mode, switch the two file options back: `main.c` included, `main_tilt.c` excluded.

## Implemented Features

- OLED initialization and game rendering.
- Main menu with difficulty selection.
- Button-controlled Snake movement using K1-K4.
- Debounced button scanning with press-event latching.
- Pause and resume during gameplay using `K1+K2`; `K3+K4` is also accepted as a backup combo.
- Snake movement, food generation, scoring, growth, wall collision, and self-collision.
- Slower movement speeds for the small OLED map:
  - Easy: 2500 ms per cell
  - Normal: 1500 ms per cell
  - Hard: 500 ms per cell
- LCD status output for score and temperature.
- Non-blocking DS1820 temperature service integrated into the main loop.
- MPU6500 and tilt-input modules prepared for optional control.

## Tilt Control

Tilt control is encapsulated but disabled in the normal entry.

Key files:

```text
Miniproj/src/mpu6500.c
Miniproj/src/tilt_input.c
Miniproj/src/hfiles/mpu6500.h
Miniproj/src/hfiles/tilt_input.h
```

The tilt module supports:

- MPU6500 initialization.
- `WHO_AM_I` detection.
- Accelerometer raw reading.
- Automatic calibration.
- Offset removal.
- Simple low-pass filtering.
- Dead-zone and release-zone handling.
- Direction output as `DIR_UP`, `DIR_DOWN`, `DIR_LEFT`, or `DIR_RIGHT`.

To run the tilt-enabled version, use `main_tilt.c` as the only active entry file, or call:

```c
App_MainWithTilt(1U);
```

Keep the board still and level for the first second after startup because tilt calibration samples are collected during that time.

## Important Source Files

```text
Miniproj/src/main.c              Formal default entry point.
Miniproj/src/main_tilt.c         Tilt-control test entry point, excluded by default.
Miniproj/src/app.c               Main application state machine, input scheduling, LCD update, temperature service, tilt switch.
Miniproj/src/menu.c              OLED menu rendering and difficulty selection.
Miniproj/src/snake_game.c        Snake game engine.
Miniproj/src/snake_render.c      OLED Snake rendering.
Miniproj/src/temp_sensor.c       DS1820 one-wire temperature state machine.
Miniproj/src/mpu6500.c           MPU6500 register-level driver.
Miniproj/src/tilt_input.c        Tilt calibration, filtering, dead-zone, and direction generation.
Miniproj/src/utils.c             Utility functions.
```

## Build Notes

Open `Miniproj/SnakeGame.uvprojx` in Keil uVision and build target `SnakeGame_Release`.

Do not submit generated build output folders unless specifically required:

```text
Miniproj/Objects/
Miniproj/Listings/
```

Do not submit Keil user layout files:

```text
*.uvguix.*
```

## Current Limitations

- Tilt mode is implemented as a testable module and entry path, but it is not yet integrated into the OLED menu as a user-selectable mode.
- MPU tilt direction signs may need adjustment after hardware testing because the physical mounting orientation determines whether positive X/Y should map to left/right/up/down.
- The UI still needs final report/demo polish: startup screen, group information screen, calibration screen, and sensor-monitor state can be expanded later.
