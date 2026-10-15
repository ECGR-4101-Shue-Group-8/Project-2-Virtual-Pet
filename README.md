# Project-2-Virtual-Pet
Design and build a Tamagotchi-style virtual pet that integrates core embedded systems concepts, including a GUI, animated interactions, changing internal states, and sensor-based controls. Each team creates a unique pet with its own look and behavior.
\




NEEDED:

pip install pillow 


RUN:

python3 tools/png_to_rgb565.py \
assets/source/IDLE_MONKEY.png \
Core/Src/idle_pixels.c \
idle_pixels 4 1

python3 tools/png_to_rgb565.py \
assets/source/FEED_MONKEY.png \
Core/Src/feed_pixels.c \
feed_pixels 4 1

python3 tools/png_to_rgb565.py \
assets/source/RUNAWAY_MONKEY.png \
Core/Src/runaway_pixels.c \
runaway_pixels 4 1

python3 tools/png_to_rgb565.py \
assets/source/DEAD_MONKEY.png \
Core/Src/dead_pixels.c \
dead_pixels 1 1
