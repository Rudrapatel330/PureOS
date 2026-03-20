#ifndef APPS_H
#define APPS_H

typedef struct {
  const char *name;
  void (*launch)(void);
} app_entry_t;

extern const app_entry_t installed_apps[];
extern const int installed_apps_count;

#endif
