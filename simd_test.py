eff_a = 100
inv_a = 155
def pmullw(a, b):
    return (a * b) & 0xFFFF

src_mul = pmullw(212, 100)
tgt_mul = pmullw(34, 155)
padd = (src_mul + tgt_mul) & 0xFFFF
res_simd = padd >> 8
print(f'G Expected: 103')
print(f'G SIMD Padd/Shift: {res_simd}')
