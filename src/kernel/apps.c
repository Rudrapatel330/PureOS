#include "../include/apps.h"
#include <stddef.h>

// Forward declarations of init functions
extern void terminal_init();
extern void calculator_init();
extern void editor_init();
extern void paint_init();
extern void explorer_init();
extern void taskmgr_init();
extern void browser_init(void);
extern void videoplayer_init(const char *path);
extern void settings_init();
extern void pdfreader_init();
extern void photos_init();
extern void mail_app_init();

// Wrapper for videoplayer to match void (*launch)(void)
static void videoplayer_launch() { videoplayer_init(NULL); }

const app_entry_t installed_apps[] = {
    {"Terminal", terminal_init}, {"Calculator", calculator_init},
    {"Editor", editor_init},     {"Paint", paint_init},
    {"Files", explorer_init},    {"Task Manager", taskmgr_init},
    {"Browser", browser_init},   {"Video Player", videoplayer_launch},
    {"Settings", settings_init}, {"PDF Reader", pdfreader_init},
    {"Photos", photos_init},     {"Mail", mail_app_init},
};

const int installed_apps_count =
    sizeof(installed_apps) / sizeof(installed_apps[0]);
