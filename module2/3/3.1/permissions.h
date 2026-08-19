#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stdbool.h>
#include <sys/types.h>

bool parse_octal_permissions(const char *text, mode_t *mode);
bool parse_letter_permissions(const char *text, mode_t *mode);
void format_letter_permissions(mode_t mode, char output[10]);
void format_binary_permissions(mode_t mode, char output[10]);
bool apply_permission_command(mode_t current, const char *command,
                              mode_t *result);

#endif
