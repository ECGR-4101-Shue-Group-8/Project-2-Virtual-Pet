from PIL import Image
import sys


def rgb888_to_rgb565(r, g, b):
    return (
        ((r >> 3) << 11)
        | ((g >> 2) << 5)
        | (b >> 3)
    )


def convert_sprite_sheet(
    input_file,
    output_file,
    array_name,
    columns,
    rows
):
    original = Image.open(input_file).convert("RGBA")

    # Flatten transparency onto a black background.
    background = Image.new(
        "RGBA",
        original.size,
        (0, 0, 0, 255)
    )

    background.alpha_composite(original)

    image = background.convert("RGB")

    width, height = image.size

    if width % columns != 0:
        raise ValueError("Width is not divisible by columns")

    if height % rows != 0:
        raise ValueError("Height is not divisible by rows")

    frame_width = width // columns
    frame_height = height // rows
    frame_count = columns * rows

    pixels = []

    # Store complete frames consecutively.
    for frame_row in range(rows):
        for frame_col in range(columns):

            start_x = frame_col * frame_width
            start_y = frame_row * frame_height

            for y in range(frame_height):
                for x in range(frame_width):

                    r, g, b = image.getpixel(
                        (start_x + x, start_y + y)
                    )

                    pixels.append(
                        rgb888_to_rgb565(r, g, b)
                    )

    with open(output_file, "w") as f:
        f.write("#include <stdint.h>\n\n")

        f.write(
            f"const uint16_t {array_name}[] = {{\n"
        )

        for i, pixel in enumerate(pixels):
            f.write(f"0x{pixel:04X}")

            if i != len(pixels) - 1:
                f.write(",")

            if (i + 1) % 12 == 0:
                f.write("\n")
            else:
                f.write(" ")

        f.write("\n};\n")

    print(f"Frames: {frame_count}")
    print(f"Frame size: {frame_width}x{frame_height}")
    print(f"Total pixels: {len(pixels)}")
    print(f"Generated: {output_file}")


if __name__ == "__main__":

    if len(sys.argv) != 6:
        print(
            "Usage: python png_to_rgb565.py "
            "<input.png> <output.c> "
            "<array_name> <columns> <rows>"
        )
        sys.exit(1)

    convert_sprite_sheet(
        sys.argv[1],
        sys.argv[2],
        sys.argv[3],
        int(sys.argv[4]),
        int(sys.argv[5])
    )
