section .rodata

global wallpaper_png_data
global wallpaper_png_size
global wallpaper_bmp_data
global wallpaper_bmp_size

wallpaper_png_data:
    incbin "src/fs/wallpaper.png"
wallpaper_png_end:
wallpaper_png_size: dd wallpaper_png_end - wallpaper_png_data

wallpaper_bmp_data:
    incbin "src/fs/wallpaper.bmp"
wallpaper_bmp_end:
wallpaper_bmp_size: dd wallpaper_bmp_end - wallpaper_bmp_data
