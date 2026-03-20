.section .rodata
.global wallpaper_png_data
.global wallpaper_png_size

wallpaper_png_data:
.incbin "src/fs/wallpaper_ultra.png"
wallpaper_png_end:

.align 4
wallpaper_png_size:
.int wallpaper_png_end - wallpaper_png_data

# Fallback symbols for compatibility
.global wallpaper_bmp_data
.global wallpaper_bmp_size
wallpaper_bmp_data:
.int 0
wallpaper_bmp_size:
.int 0
