#include "permissions.h"

#include <stddef.h>
#include <string.h>
#include <sys/stat.h>

static const mode_t PERMISSION_BITS[9] = {
    S_IRUSR, S_IWUSR, S_IXUSR,
    S_IRGRP, S_IWGRP, S_IXGRP,
    S_IROTH, S_IWOTH, S_IXOTH
};

static const char PERMISSION_CHARS[9] = {
    'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x'
};

bool parse_octal_permissions(const char *text, mode_t *mode)
{
    mode_t value = 0;
    size_t i;

    if (text == NULL || mode == NULL || strlen(text) != 3)
        return false;

    for (i = 0; i < 3; i++)
    {
        if (text[i] < '0' || text[i] > '7')
            return false;
        value = (value << 3) | (mode_t)(text[i] - '0');
    }

    *mode = value;
    return true;
}

bool parse_letter_permissions(const char *text, mode_t *mode)
{
    mode_t value = 0;
    size_t i;

    if (text == NULL || mode == NULL || strlen(text) != 9)
        return false;

    for (i = 0; i < 9; i++)
    {
        if (text[i] == PERMISSION_CHARS[i])
            value |= PERMISSION_BITS[i];
        else if (text[i] != '-')
            return false;
    }

    *mode = value;
    return true;
}

void format_letter_permissions(mode_t mode, char output[10])
{
    size_t i;

    for (i = 0; i < 9; i++)
        output[i] = (mode & PERMISSION_BITS[i]) != 0
                        ? PERMISSION_CHARS[i]
                        : '-';
    output[9] = '\0';
}

void format_binary_permissions(mode_t mode, char output[10])
{
    size_t i;

    for (i = 0; i < 9; i++)
        output[i] = (mode & PERMISSION_BITS[i]) != 0 ? '1' : '0';
    output[9] = '\0';
}

static mode_t expand_rights(mode_t scope, mode_t rights)
{
    mode_t expanded = 0;

    if ((scope & 0700) != 0)
        expanded |= rights << 6;
    if ((scope & 0070) != 0)
        expanded |= rights << 3;
    if ((scope & 0007) != 0)
        expanded |= rights;

    return expanded;
}

static bool parse_scope(const char **position, mode_t *scope)
{
    bool seen_u = false;
    bool seen_g = false;
    bool seen_o = false;
    bool seen_a = false;

    *scope = 0;

    while (**position == 'u' || **position == 'g' ||
           **position == 'o' || **position == 'a')
    {
        if (**position == 'u')
        {
            if (seen_u || seen_a)
                return false;
            seen_u = true;
            *scope |= 0700;
        }
        else if (**position == 'g')
        {
            if (seen_g || seen_a)
                return false;
            seen_g = true;
            *scope |= 0070;
        }
        else if (**position == 'o')
        {
            if (seen_o || seen_a)
                return false;
            seen_o = true;
            *scope |= 0007;
        }
        else
        {
            if (seen_a || seen_u || seen_g || seen_o)
                return false;
            seen_a = true;
            *scope = 0777;
        }
        (*position)++;
    }

    return *scope != 0;
}

static bool parse_rights(const char **position, char operation,
                         mode_t *rights)
{
    bool seen_r = false;
    bool seen_w = false;
    bool seen_x = false;

    *rights = 0;

    while (**position == 'r' || **position == 'w' || **position == 'x')
    {
        if (**position == 'r')
        {
            if (seen_r)
                return false;
            seen_r = true;
            *rights |= 04;
        }
        else if (**position == 'w')
        {
            if (seen_w)
                return false;
            seen_w = true;
            *rights |= 02;
        }
        else
        {
            if (seen_x)
                return false;
            seen_x = true;
            *rights |= 01;
        }
        (*position)++;
    }

    return *rights != 0 || operation == '=';
}

static bool reject_permission_command(mode_t current, mode_t *result)
{
    *result = current & 0777;
    return false;
}

bool apply_permission_command(mode_t current, const char *command,
                              mode_t *result)
{
    const char *position = command;
    mode_t candidate = current & 0777;

    if (command == NULL || result == NULL || *command == '\0')
        return false;

    while (*position != '\0')
    {
        mode_t scope = 0;
        mode_t rights = 0;
        char operation;
        mode_t expanded;

        if (!parse_scope(&position, &scope))
            return reject_permission_command(current, result);
        if (*position != '+' && *position != '-' && *position != '=')
            return reject_permission_command(current, result);

        operation = *position;
        position++;

        if (!parse_rights(&position, operation, &rights))
            return reject_permission_command(current, result);
        if (*position != '\0' && *position != ',')
            return reject_permission_command(current, result);

        expanded = expand_rights(scope, rights);
        if (operation == '+')
            candidate |= expanded;
        else if (operation == '-')
            candidate &= ~expanded;
        else
            candidate = (candidate & ~scope) | expanded;

        if (*position == ',')
        {
            position++;
            if (*position == '\0')
                return reject_permission_command(current, result);
        }
    }

    *result = candidate & 0777;
    return true;
}
