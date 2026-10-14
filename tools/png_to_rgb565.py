from PIL import Image
import sys


def rgb888_to_rgb565(r, g, b):
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F

    return (r5 << 11) | (g6 << 5) | b5


def convert_sprite_sheet(input_file, output_file):
    image = Image.open(input_file).convert("RGB")

    width, height = image.size

    # Our Piskel sheets are 4 frames in one horizontal row.
    frame_count = 4

    if width % frame_count != 0:
        raise ValueError("Image width must be divisible by 4.")

    frame_width = width // frame_count
    frame_height = height

    print(f"Image: {width}x{height}")
    print(f"Frames: {frame_count}")
    print(f"Frame size: {frame_width}x{frame_height}")

    pixels = []

      # Store each frame contiguously:
    #
    # frame 0 pixels
    # frame 1 pixels
    # frame 2 pixels
    # frame 3 pixels
    #
    # This makes the STM32 code much simpler than storing
    # the original 4x1 image layout.
    
    for frame in range(frame_count):

        start_x = frame * frame_width

        for y in range(frame_height):
            for x in range(frame_width):

                r, g, b = image.getpixel((start_x + x, y))

                pixels.append(rgb888_to_rgb565(r, g, b))

    with open(output_file, "w") as f:

        f.write('#include <stdint.h>\n\n')

        f.write("const uint16_t idle_pixels[] =\n")
        f.write("{\n")

        for i, pixel in enumerate(pixels):

            f.write(f"0x{pixel:04X}")

            if i != len(pixels) - 1:
                f.write(",")

            if (i + 1) % 12 == 0:
                f.write("\n")
            else:
                f.write(" ")

        f.write("\n};\n")

    print(f"Wrote {len(pixels)} RGB565 pixels.")
    print(f"Output: {output_file}")


if __name__ == "__main__":

    if len(sys.argv) != 3:
        print(
            "Usage: python png_to_rgb565.py "
            "<input.png> <output.c>"
        )
        sys.exit(1)

    convert_sprite_sheet(
        sys.argv[1],
        sys.argv[2]
    )
