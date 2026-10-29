# Monkey Pal — Virtual Pet

**ECGR 4101: Advanced Embedded Systems**  
**UNC Charlotte | Group 8 | Project 2**

## Overview

Monkey Pal is a Tamagotchi-style virtual pet developed using an STM32 microcontroller and TFT LCD. Players interact with an animated monkey while managing its hunger and happiness through different activities.

**Repository:** [Project-2-Virtual-Pet](https://github.com/ECGR-4101-Shue-Group-8/Project-2-Virtual-Pet)

## Features

- **Animated Pet:** Custom monkey sprites for idle, feeding, death, and running away.
- **Pet Statistics:** Hunger and happiness values ranging from 0–100.
- **Player Actions:** Feeding and two additional configurable interactions.
- **Graphical Interface:** LCD menu navigated using the physical joystick.
- **State Machine:** Controls pet behavior, interactions, and state transitions.
- **Hardware Timing:** TIM6 generates a 1 Hz interrupt for statistics.
- **Animation Scheduling:** Updates animation frames every 100 ms.
- **Consequences:** The pet can die or run away when its needs are neglected.
- **Restart System:** Allows starting a new pet following a terminal state.

## Hardware

| Component | Purpose |
|---|---|
| STM32 NUCLEO-L496ZG-P | Main microcontroller |
| X-NUCLEO-GFX01M2 | 240 × 320 TFT LCD and joystick |
| ST-LINK USB | Programming, debugging, and power |
| Analog input device | Additional pet interaction (TBD) |
| I2C/UART/SPI peripheral | Additional pet interaction (TBD) |

The project uses the instructor-provided HAL/CubeMX starter project from `Lab1_NUCLEO_L496.zip`.

## Project Structure

```text
Project-2-Virtual-Pet/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── clock.h
│   │   ├── pet_state.h
│   │   ├── pet_anim.h
│   │   ├── display.h
│   │   ├── sprites.h
│   │   └── ui.h
│   │
│   └── Src/
│       ├── main.c
│       ├── clock.c
│       ├── pet_state.c
│       ├── pet_anim.c
│       ├── display.c
│       ├── sprites.c
│       ├── ui.c
│       ├── idle_pixels.c
│       ├── feed_pixels.c
│       ├── runaway_pixels.c
│       └── dead_pixels.c
│
├── Drivers/
│   └── BSP/
│       └── GFX01M2/
│           ├── ili9341.c
│           └── ili9341.h
│
├── assets/
│   └── source/
│       ├── IDLE_MONKEY.png
│       ├── FEED_MONKEY.png
│       ├── RUNAWAY_MONKEY.png
│       └── DEAD_MONKEY.png
│
├── tools/
│   └── png_to_rgb565.py
│
├── CMakeLists.txt
└── README.md
```

## Software Modules

| File | Responsibility |
|---|---|
| `main.c` | Hardware initialization and main execution loop |
| `clock.c/.h` | TIM6 stat timer (1 Hz) and `clock_millis()` using `HAL_GetTick()` |
| `pet_state.c/.h` | Pet state machine, hunger/happiness statistics, and user actions |
| `pet_anim.c/.h` | Calls the corresponding LCD animation for the active state |
| `display.c/.h` | RGB565 sprite rendering |
| `sprites.c/.h` | Sprite sheet definitions and frame information |
| `ui.c/.h` | Menu navigation, statistics, and graphical interface |
| `*_pixels.c` | RGB565 pixel arrays generated from PNG artwork |

---

## States and Clock

The States and Clock subsystem manages the pet state machine, hunger and happiness statistics, the 1 Hz stat timer, and animation callbacks.

It was developed for the HAL/CubeMX project in `Lab1_NUCLEO_L496.zip`.

**Development status:** The States and Clock subsystem has been test-built against the starter project but has not yet been tested on a physical board.

### Pet States

| State | Description |
|---|---|
| Idle | Default state; the pet waits for interactions |
| Feed | Plays feeding animation and modifies statistics |
| Action A | First additional interaction |
| Action B | Second additional interaction |
| Dead | Terminal state when the pet's needs are neglected |
| Runaway | Additional terminal behavior requiring integration/verification |

Actions last approximately 2 seconds. Requests received while the pet is busy performing an action are ignored.

### Clock

The project retains the existing `SystemClock_Config()` in `main.c`.

- **System clock:** MSI → PLL, approximately 56.8 MHz.
- **TIM6:** Generates one interrupt per second.
- **`clock_millis()`:** Wraps `HAL_GetTick()` to provide elapsed time in milliseconds.
- **Animation interval:** 100 ms (`PET_ANIM_FRAME_MS`).

TIM6 determines the actual bus clock when initialized so the one-second tick remains correct if the system clock configuration is changed.

TIM6 is configured directly through its registers. The HAL TIM module does not need to be enabled.

**Important:** Do not assign TIM6 to another peripheral or function in the `.ioc` file.

### Pet Statistics

Both statistics are limited to values between 0 and 100.

| Statistic | Meaning | Automatic change |
|---|---|---|
| Hunger | 0 = full, 100 = starving | +1 every 5 seconds |
| Happiness | 100 = happy, 0 = miserable | −1 every 8 seconds |

When hunger exceeds 70, happiness decreases twice as fast.

### User Actions

| Action | Hunger | Happiness |
|---|---|---|
| Feed | −25 | +5 |
| Feed while already full | No reduction | −10 |
| Action A | — | +15 |
| Action B | — | +15 |

Action A and Action B are placeholders until their final interactions are implemented.

All statistic changes are clamped to the 0–100 range.

**Current state-machine behavior:** The pet dies when hunger reaches 100 or happiness reaches 0. The required runaway outcome must be verified or implemented separately.

The values and intervals can be adjusted through the `#define` constants in `pet_state.c`.

---


| Event | Purpose |
|---|---|
| `PET_EVENT_FEED` | Start feeding |
| `PET_EVENT_ACTION_A` | Start additional action A |
| `PET_EVENT_ACTION_B` | Start additional action B |
| `PET_EVENT_RESET` | Start a new pet |

Requests made while the pet is busy with an action are ignored.

---

## Animations

Each pet state is associated with an LCD animation function.

| State | Function |
|---|---|
| Idle | `anim_idle(frame)` |
| Feed | `anim_feed(frame)` |
| Action A | `anim_action_a(frame)` |
| Action B | `anim_action_b(frame)` |
| Dead | `anim_dead(frame)` |
| Runaway | `anim_runaway(frame)` (if integrated) |

### Animation Timing

- `frame == 0` indicates that a state was just entered.
- Subsequent frames are updated every 100 ms.
- Each animation function draws one frame and returns quickly.
- For a four-frame sprite, select the image using `frame % 4`.
- To animate at half the speed, use `(frame / 2) % 4`.

Example:

```c
void anim_idle(uint32_t frame)
{
    uint8_t sprite_frame = frame % 4;

    display_draw_sprite_frame(
        &sprite_idle,
        sprite_frame,
        88,
        116
    );
}
```

Animation functions can use `pet_get_stats()` to choose different artwork based on the monkey's current hunger or happiness.

The existing `pet_anim.c` module dispatches animation calls according to the state machine.

To integrate custom animations, implement functions with the expected names or update the `state_anims` dispatch table in `pet_anim.c`.

---

## Sprite Conversion

The STM32 cannot directly render PNG files using the current LCD driver. Artwork must first be converted into RGB565 pixel arrays.

### Requirements

- Python 3
- Pillow image processing library

Install Pillow:

```bash
pip install pillow
```

Alternatively:

```bash
python3 -m pip install pillow
```

### Sprite Layout

| File | Frames | Arrangement |
|---|---|---|
| `IDLE_MONKEY.png` | 4 | 4 × 1 |
| `FEED_MONKEY.png` | 4 | 4 × 1 |
| `RUNAWAY_MONKEY.png` | 4 | 4 × 1 |
| `DEAD_MONKEY.png` | 1 | 1 × 1 |

Each four-frame sprite sheet is arranged horizontally.

### Conversion Commands

Run the following commands from the repository root directory.

**Idle:**

```bash
python3 tools/png_to_rgb565.py \
  assets/source/IDLE_MONKEY.png \
  Core/Src/idle_pixels.c \
  idle_pixels 4 1
```

**Feed:**

```bash
python3 tools/png_to_rgb565.py \
  assets/source/FEED_MONKEY.png \
  Core/Src/feed_pixels.c \
  feed_pixels 4 1
```

**Runaway:**

```bash
python3 tools/png_to_rgb565.py \
  assets/source/RUNAWAY_MONKEY.png \
  Core/Src/runaway_pixels.c \
  runaway_pixels 4 1
```

**Dead:**

```bash
python3 tools/png_to_rgb565.py \
  assets/source/DEAD_MONKEY.png \
  Core/Src/dead_pixels.c \
  dead_pixels 1 1
```

On Windows PowerShell, use each command on a single line instead of using the backslash continuation character.

### Generated Output

Each output file contains a constant RGB565 pixel array:

```c
#include <stdint.h>

const uint16_t idle_pixels[] = {
    // RGB565 pixel values
};
```

The arrays are compiled into the STM32 firmware.

An uncompressed 64 × 64 RGB565 image occupies 8,192 bytes. A four-frame sprite sheet with 64 × 64 frames requires 32,768 bytes.

Keep the original PNG files in `assets/source/` for future modifications.

---

## User Interface and Controls

The X-NUCLEO-GFX01M2 display has a built-in joystick for menu navigation.

| Input | Function |
|---|---|
| Joystick Up | Previous menu option |
| Joystick Down | Next menu option |
| Joystick Center | Open menu / select action |
| Joystick Left | Return to main screen |
| Center after terminal state | Restart pet |

The interface displays the monkey's current animation and its hunger and happiness meters.

The intended menu provides access to Feed, Play, and Rest, with the final actions subject to the team's peripheral integration.

---

## Building and Running

1. Clone the repository:

   ```bash
   git clone https://github.com/ECGR-4101-Shue-Group-8/Project-2-Virtual-Pet.git
   ```

2. Open the STM32 project in VS Code.
3. Verify the required HAL and BSP files are present.
4. Connect the LCD to the NUCLEO board using the instructor-specified alignment.
5. Generate the RGB565 sprite arrays.
6. Verify all application sources are included in `CMakeLists.txt`.
7. Build the project.
8. Connect the NUCLEO board using ST-LINK USB.
9. Flash the firmware.
10. Test joystick navigation, statistics, interactions, animations, and terminal states.

## Development Status

**Work in progress**

The States and Clock subsystem has been test-built with the course starter project but has not yet been verified on physical hardware. Could not get code to run with LCD. Tried troubleshooting by running Lab1 again. Worked without LCD attached, but when attached computer did not recognize device.

Integration and testing tasks include:

- [ ] Verify LCD initialization and bitmap rendering.
- [ ] Confirm all sprite animations display correctly.
- [ ] Test joystick navigation and action selection.
- [ ] Verify TIM6 timing and statistic changes.
- [ ] Confirm hunger and happiness boundaries.
- [ ] Verify death and runaway behaviors separately.
- [ ] Integrate an additional analog input device.
- [ ] Integrate an I2C, UART, or SPI peripheral.
- [ ] Validate operation on the NUCLEO-L496ZG-P hardware.

## Team Contributions

| Team Member | Contributions |
|---|---|
| [Josh] | States and Clock |
| [Omari] | Visuals and UI |
| [Matthew] | Hardware and peripheral integration |
| [Nick Rudd] | None|

Individual contributions and development progress are documented through GitHub commit history.

## AI Tools

VsCode Copilot, ChatGPT, and ClaudeCode was used to assist with software organization, code development suggestions, STM32 integration, and documentation. All generated code is subject to team review and testing.

