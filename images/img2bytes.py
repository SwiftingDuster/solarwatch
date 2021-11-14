import os
import sys

from PIL import Image


def main():
    if len(sys.argv) > 1:
        path = sys.argv[1]
        if not os.path.isfile(path):
            print("Path is invalid.")
            exit(0)
    else:
        print("Usage: python3 img2bytes.py <path>")
        exit(0)

    image = Image.open(path)
    pixels = image.load()
    width, height = image.size
    hexList = []

    # Convert 3 byte RGB (RRRRRRRR GGGGGGGG BBBBBBBB) to 1 byte RGB (BBB GGG RR)
    for h in range(height):
        for w in range(width):
            # May have extra data (e.g. alpha), 4th var stores the rest
            pix = pixels[w, h]
            red, green, blue, _ = (
                pix[0],
                pix[1],
                pix[2],
                pix[3:],
            )

            b = 0b11100000 & blue
            g = 0b00011100 & (green >> 3)
            r = 0b00000011 & (red >> 6)

            byteRGB = b | g | r
            hexList.append(byteRGB)

    # Print rows of bytes with length equal to image width
    # E.g. A 32x32 image would print 32 bytes/row
    for index, byte in enumerate(hexList):
        if byte < 0x10:
            # Pad with zero
            print("0x{:02x}".format(byte), end="")
        else:
            print(hex(byte), end="")
        if index < len(hexList) - 1:
            print(", ", end="")
        if index % width == width - 1:
            print()

    image.close()


main()
