#ifndef UI_H
#define UI_H

#include "phonebook.h"

PhonebookStatus uiRun(Phonebook *phonebook, const char *contacts_file);
const char *uiPhonebookStatusMessage(PhonebookStatus status);

#endif
