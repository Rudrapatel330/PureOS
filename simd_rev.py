eff_a = 100
for test_inv_a in range(-155, 300):
   for test_eff_a in range(-128, 255):
        # We got 249(F9), 69, 255
        g_src = 212
        g_tgt = 34
        g_simd = (((g_src * test_eff_a) & 0xFFFF) + ((g_tgt * test_inv_a) & 0xFFFF)) & 0xFFFF
        g_out = g_simd >> 8

        b_src = 49
        b_tgt = 51
        b_simd = (((b_src * test_eff_a) & 0xFFFF) + ((b_tgt * test_inv_a) & 0xFFFF)) & 0xFFFF
        b_out = b_simd >> 8

        if g_out == 69 and b_out == 255:
            print(f'Match found: eff_a={test_eff_a}, inv_a={test_inv_a}')

