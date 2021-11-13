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
            r, g, b, _ = (
                pix[0],
                pix[1],
                pix[2],
                pix[3:],
            )

            blue = b & 0b11100000
            green = (g >> 3) & 0b11100
            red = (r >> 6) & 0b11
            byteRGB = blue | green | red
            hexList.append(byteRGB)

    # Print rows of bytes with length equal to image width
    # E.g. A 32x24 image would print 32 bytes/row
    for index, byte in enumerate(hexList):
        print(hex(byte), end="")
        if index < len(hexList) - 1:
            print(", ", end="")
        if index % 32 == 31:
            print()

    image.close()


main()
