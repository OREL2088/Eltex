#include "phonebook.h"
#include "storage.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CONTACTS_FILE "contacts.txt"

int main(void)
{
    Phonebook phonebook;
    PhonebookStatus status;
    size_t loaded = 0;

    status = phonebookInit(&phonebook);
    if (status != PHONEBOOK_OK)
        return EXIT_FAILURE;
    srand((unsigned int)time(NULL));
    status = storageLoad(&phonebook, CONTACTS_FILE, &loaded);
    if (status != PHONEBOOK_OK && status != PHONEBOOK_IO_ERROR) {
        fprintf(stderr, "Не удалось загрузить справочник: %s.\n",
                uiStatusMessage(status));
    }
    status = uiRun(&phonebook, CONTACTS_FILE);
    phonebookClear(&phonebook);
    return status == PHONEBOOK_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
