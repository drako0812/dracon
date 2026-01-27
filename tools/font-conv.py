from typing import cast

from PIL import Image

with Image.open("../font.png") as in_img:
    # in_img.show()
    print(in_img.format, in_img.size, in_img.mode)
    newlen_bits = in_img.size[0] * in_img.size[1]
    print(f"New Length in Bits: {newlen_bits}")
    newlen_bytes = int(newlen_bits / 8)
    print(f"New Length in Bytes: {newlen_bytes}")

    data_bits: list[int] = []
    data_bytes: bytes = b""

    for c in range(0, 256):
        # c is the character index
        xtile = c % 16
        ytile = c // 16
        xpix = xtile * 8
        ypix = ytile * 8
        # print(f"c:{c}, xt:{xtile}, yt:{ytile}, xp:{xpix}, yp:{ypix}")

        for yy in range(ypix, ypix + 8):
            for xx in range(xpix, xpix + 8):
                pix = cast(tuple, in_img.getpixel((xx, yy)))
                # print(f"({xx}, {yy}, {pix[0]})", end="\t")
                # print(pix)
                # print(pix[0], end=", ")
                if pix[0] == 0:
                    data_bits.append(0)
                else:
                    data_bits.append(1)
        # print()

    # print(data_bits)
    # print(f"len(data_bits) = {len(data_bits)}, newlen_bits = {newlen_bits}")

    for idx in range(0, len(data_bits), 8):
        buf = 0
        for idx2 in range(idx, idx + 8):
            buf = (buf << 1) | (data_bits[idx2] & 0x1)
        data_bytes += buf.to_bytes()

    # print(data_bytes)

    #with open("../src/font.dat", mode="w+b") as ofile:
    #    ofile.write(data_bytes)

    with open("../src/console/font.dat", mode="w") as ofile:
        for b in data_bytes:
            ofile.write(f"{int(b)}, ")
