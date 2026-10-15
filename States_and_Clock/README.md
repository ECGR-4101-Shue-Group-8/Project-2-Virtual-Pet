# States and Clock

Pet state machine, hunger/happiness/energy stats, the 1 Hz stat timer, and the hook that plays an LCD animation for each state. Written for the HAL/CubeMX project in `Lab1_NUCLEO_L496.zip`. It has been test-built against that project but not yet run on a board.

| File | What it does |
|---|---|
| `clock.c/.h` | TIM6 stat timer (1 tick per second) and `clock_millis()` (wraps `HAL_GetTick()`) |
| `pet_state.c/.h` | States: Idle, Feed, Play, Sleep, Dead, Runaway. Hunger, happiness and energy stats |
| `pet_anim.c/.h` | Calls the matching LCD animation for whichever state the pet is in |

## Adding it to the project

1. Copy `Inc/*` into `Core/Inc/` and `Src/*` into `Core/Src/`.
2. In the project's top-level `CMakeLists.txt`, under `# Add user sources here`, add:
   ```cmake
   Core/Src/clock.c
   Core/Src/pet_state.c
   Core/Src/pet_anim.c
   ```
3. In `main.c`:
   ```c
   #include "clock.h"
   #include "pet_state.h"

   int main(void)
   {
     HAL_Init();
     SystemClock_Config();
     MX_GPIO_Init();
     MX_SPI1_Init();
     ILI9341_Init(&hspi1);

     clock_init();   // after SystemClock_Config()
     pet_init();     // after the LCD is ready, since it draws the first idle frame

     while (1)
     {
       // read inputs, then e.g. pet_request(PET_EVENT_FEED);
       pet_update();
     }
   }
   ```
   Don't use `HAL_Delay()` for long waits in the main loop. Stats are counted by interrupt, so they stay correct during a delay, but buttons and animations freeze until it ends.

No CubeMX changes are needed. TIM6 is set up directly through its registers, so the HAL TIM module doesn't have to be enabled. Don't assign TIM6 to anything else in the `.ioc`.

## Clock

The system clock is still the existing `SystemClock_Config()` in `main.c` (MSI -> PLL = 56.8 MHz). TIM6 reads the real bus clock when it starts, so if the system clock is changed later, the stat tick stays at 1 second.

## Inputs (for the peripheral code)

Call these from anywhere, including interrupts:

```c
pet_request(PET_EVENT_FEED);
pet_request(PET_EVENT_PLAY);    // ignored if energy is below 15 (too tired)
pet_request(PET_EVENT_SLEEP);   // e.g. light sensor reads dark
pet_request(PET_EVENT_WAKE);    // e.g. light sensor reads bright
pet_request(PET_EVENT_RESET);   // start a new pet after it died or left
```

- Feed and Play only work from Idle. Requests made during them (2 s and 3 s) are ignored.
- While the pet is asleep, only Wake does anything.

## Animations (for the LCD code)

Each state calls one function with a frame number:

| State | Function |
|---|---|
| Idle | `anim_idle(frame)` |
| Feed | `anim_feed(frame)` |
| Play | `anim_play(frame)` |
| Sleep | `anim_sleep(frame)` |
| Dead | `anim_dead(frame)` |
| Runaway | `anim_runaway(frame)` |

- `frame == 0` means the state was just entered: clear the area and draw the first frame.
- After that, the function is called with frame 1, 2, 3… every 100 ms (`PET_ANIM_FRAME_MS`).
- `anim_runaway` should play RUNAWAY_MONKEY.png once (frames 0-3), then leave the pet off screen. For example, use `frame < 4 ? frame : 3` for the sprite, or clear the screen after frame 3.
- For a looping 4-frame sprite sheet such as IDLE_MONKEY.png, draw sprite `frame % 4`. If it should animate slower than every 100 ms, use `(frame / 2) % 4`.
- Draw one frame and return quickly.
- `pet_get_stats()` gives hunger, happiness and energy, for example to show a sad or sleepy idle animation.

To connect the real animations, either write functions with these exact names (they automatically replace the empty placeholders), or put your own function names in the `state_anims` table in `pet_anim.c`.

## Stats

All stats run from 0 to 100. All of the numbers below are `#define`s at the top of `pet_state.c`.

| Stat | Meaning | Awake | Asleep |
|---|---|---|---|
| Hunger | 0 = full, 100 = starving | +1 every 5 s | +1 every 10 s |
| Happiness | 100 = happy, 0 = miserable (pet runs away) | -1 every 8 s, faster if hungry (above 70) or tired (energy below 20) | doesn't change |
| Energy | 100 = rested, 0 = exhausted | -1 every 10 s | +1 every 2 s |

| Action | Effect |
|---|---|
| Feed | hunger -25, happiness +5. Feeding while already full costs 10 happiness instead |
| Play | happiness +20, energy -15, hunger +5. Needs at least 15 energy |
| Sleep | runs until energy reaches 100 or a Wake request comes in. Waking it early with energy below 30 costs 5 happiness |

- If energy hits 0, the pet falls asleep on its own.
- The pet dies (Dead) when hunger reaches 100.
- The pet runs away (Runaway) when happiness reaches 0. If both happen at once, Dead wins.
- After Dead or Runaway, stats stop changing until a Reset starts a new pet.
