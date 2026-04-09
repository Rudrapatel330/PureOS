#ifndef CONTACTS_H
#define CONTACTS_H

// Declare shared OS contact functions implemented in chat.c
void contacts_add(const char *number, const char *name);
const char* contacts_get_name(const char *number);

#endif
