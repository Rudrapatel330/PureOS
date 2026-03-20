/*
 * wallpaper_blobs.c - Embedded binary blobs for wallpapers
 */

__asm__(".section .rodata\n"
        ".global wallpaper_png_data\n"
        "wallpaper_png_data:\n"
        ".incbin \"src/fs/wallpaper.png\"\n"
        "wallpaper_png_end:\n"
        ".global wallpaper_png_size\n"
        "wallpaper_png_size:\n"
        ".long wallpaper_png_end - wallpaper_png_data\n"
        "\n"
        ".global wallpaper_bmp_data\n"
        "wallpaper_bmp_data:\n"
        ".incbin \"src/fs/wallpaper.bmp\"\n"
        "wallpaper_bmp_end:\n"
        ".global wallpaper_bmp_size\n"
        "wallpaper_bmp_size:\n"
        ".long wallpaper_bmp_end - wallpaper_bmp_data\n");
