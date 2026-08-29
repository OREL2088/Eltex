#ifndef STORAGE_H
#define STORAGE_H

#include "phonebook.h"

PhonebookStatus storageLoad(Phonebook *phonebook, const char *filename,
                            size_t *loaded_count);
PhonebookStatus storageSave(const Phonebook *phonebook, const char *filename);

#endif
