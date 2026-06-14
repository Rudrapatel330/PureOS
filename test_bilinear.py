def bilinear(fx):
    x_diff = (fx >> 8) & 0xFF
    x_inv = 256 - x_diff
    w00 = (x_inv * 256) >> 8
    w01 = (x_diff * 256) >> 8
    
    p00 = 255
    p01 = 0
    r = (p00 * w00 + p01 * w01) >> 8
    print(f"fx={fx} x_diff={x_diff} w00={w00} w01={w01} r={r}")

for i in range(10):
    fx = int(i * (65536 / 10))
    bilinear(fx)
