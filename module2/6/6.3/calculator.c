#include "calculator.h"
#include "plugin.h"

#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

struct CalculatorOperation
{
    void *library;
    const CalculatorPlugin *plugin;
};

static char *makePath(const char *directory, const char *file)
{
    size_t directory_length = strlen(directory);
    size_t file_length = strlen(file);
    int add_slash = directory_length > 0U &&
                    directory[directory_length - 1U] != '/';
    char *path = malloc(directory_length + (size_t)add_slash + file_length + 1U);

    if (path != NULL)
    {
        memcpy(path, directory, directory_length);
        if (add_slash)
            path[directory_length++] = '/';
        memcpy(path + directory_length, file, file_length + 1U);
    }
    return path;
}

static int isLibrary(const char *name)
{
    size_t length = strlen(name);
    return length > 3U && strcmp(name + length - 3U, ".so") == 0;
}

static int compareOperations(const void *left, const void *right)
{
    const CalculatorOperation *first = left;
    const CalculatorOperation *second = right;

    if (first->plugin->order < second->plugin->order)
        return -1;
    if (first->plugin->order > second->plugin->order)
        return 1;
    return 0;
}

CalculatorStatus calculatorClear(Calculator *calculator)
{
    size_t index;

    if (calculator == NULL)
        return CALCULATOR_INVALID_ARGUMENT;
    for (index = 0U; index < calculator->count; ++index)
        dlclose(calculator->operations[index].library);
    free(calculator->operations);
    calculator->operations = NULL;
    calculator->count = 0U;
    return CALCULATOR_OK;
}

CalculatorStatus calculatorInit(Calculator *calculator,
                                const char *plugin_directory)
{
    DIR *directory;
    struct dirent *entry;

    if (calculator == NULL || plugin_directory == NULL)
        return CALCULATOR_INVALID_ARGUMENT;
    calculator->operations = NULL;
    calculator->count = 0U;

    directory = opendir(plugin_directory);
    if (directory == NULL)
        return CALCULATOR_DIRECTORY_ERROR;

    while ((entry = readdir(directory)) != NULL)
    {
        CalculatorOperation *resized;
        const CalculatorPlugin *plugin;
        char *path;
        void *library;

        if (!isLibrary(entry->d_name))
            continue;
        path = makePath(plugin_directory, entry->d_name);
        if (path == NULL)
        {
            closedir(directory);
            calculatorClear(calculator);
            return CALCULATOR_NO_MEMORY;
        }
        library = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        free(path);
        if (library == NULL)
            continue;

        plugin = (const CalculatorPlugin *)dlsym(library,
                                                 "calculator_plugin");
        if (plugin == NULL || plugin->name == NULL || plugin->function == NULL)
        {
            dlclose(library);
            continue;
        }

        resized = realloc(calculator->operations,
                          (calculator->count + 1U) * sizeof(*resized));
        if (resized == NULL)
        {
            dlclose(library);
            closedir(directory);
            calculatorClear(calculator);
            return CALCULATOR_NO_MEMORY;
        }
        calculator->operations = resized;
        calculator->operations[calculator->count++] =
            (CalculatorOperation){library, plugin};
    }
    closedir(directory);

    if (calculator->count == 0U)
        return CALCULATOR_NO_OPERATIONS;
    qsort(calculator->operations, calculator->count,
          sizeof(*calculator->operations), compareOperations);
    return CALCULATOR_OK;
}

CalculatorStatus calculatorGetOperationName(const Calculator *calculator,
                                            size_t index, const char **name)
{
    if (calculator == NULL || name == NULL)
        return CALCULATOR_INVALID_ARGUMENT;
    if (index >= calculator->count)
        return CALCULATOR_INDEX_OUT_OF_RANGE;
    *name = calculator->operations[index].plugin->name;
    return CALCULATOR_OK;
}

CalculatorStatus calculatorExecute(const Calculator *calculator, size_t index,
                                   double first, double second,
                                   double *result)
{
    const CalculatorPlugin *plugin;

    if (calculator == NULL || result == NULL)
        return CALCULATOR_INVALID_ARGUMENT;
    if (index >= calculator->count)
        return CALCULATOR_INDEX_OUT_OF_RANGE;

    plugin = calculator->operations[index].plugin;
    if ((plugin->flags & OPERATION_SECOND_OPERAND_NONZERO) != 0U &&
        second == 0.0)
        return CALCULATOR_INVALID_OPERAND;
    *result = plugin->function(first, second);
    return CALCULATOR_OK;
}
