#ifndef STORAGE_H
#define STORAGE_H

#include "phonebook.h"

typedef enum {
    STORAGE_OK = 0,
    STORAGE_INVALID_ARGUMENT,
    STORAGE_OPEN_ERROR,
    STORAGE_READ_ERROR,
    STORAGE_WRITE_ERROR,
    STORAGE_FORMAT_ERROR,
    STORAGE_DATA_ERROR
} StorageStatus;

StorageStatus storageLoad(Phonebook *phonebook, const char *filename,
                          size_t *loaded_count);
StorageStatus storageSave(const Phonebook *phonebook, const char *filename);
const char *storageStatusMessage(StorageStatus status);

#endif
