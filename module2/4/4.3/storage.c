#include "storage.h"

#include <stdio.h>

typedef struct {
    FILE *file;
} SaveContext;

static PhonebookStatus writeContact(const Contact *contact, void *context)
{
    SaveContext *save_context = context;

    if (fprintf(save_context->file, "%d;%s;%s;%s\n", contact->id,
                contact->name, contact->surname, contact->phone) < 0)
        return PHONEBOOK_IO_ERROR;
    return PHONEBOOK_OK;
}

PhonebookStatus storageLoad(Phonebook *phonebook, const char *filename,
                            size_t *loaded_count)
{
    FILE *file;
    char line[256];
    size_t loaded = 0;
    PhonebookStatus status = PHONEBOOK_OK;

    if (phonebook == NULL || filename == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    file = fopen(filename, "r");
    if (file == NULL)
        return PHONEBOOK_IO_ERROR;

    while (fgets(line, sizeof(line), file) != NULL) {
        Contact contact;
        int consumed = 0;

        if (sscanf(line, "%d;%49[^;];%49[^;];%19[^\r\n]%n", &contact.id,
                   contact.name, contact.surname, contact.phone,
                   &consumed) != 4 ||
            (line[consumed] != '\0' && line[consumed] != '\n' &&
             !(line[consumed] == '\r' && line[consumed + 1] == '\n'))) {
            status = PHONEBOOK_FORMAT_ERROR;
            break;
        }
        status = phonebookInsert(phonebook, &contact);
        if (status != PHONEBOOK_OK)
            break;
        loaded++;
    }
    if (ferror(file))
        status = PHONEBOOK_IO_ERROR;
    if (fclose(file) != 0 && status == PHONEBOOK_OK)
        status = PHONEBOOK_IO_ERROR;
    if (loaded_count != NULL)
        *loaded_count = loaded;
    return status;
}

PhonebookStatus storageSave(const Phonebook *phonebook, const char *filename)
{
    SaveContext context;
    PhonebookStatus status;

    if (phonebook == NULL || filename == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    context.file = fopen(filename, "w");
    if (context.file == NULL)
        return PHONEBOOK_IO_ERROR;
    status = phonebookForEach(phonebook, writeContact, &context);
    if (fclose(context.file) != 0 && status == PHONEBOOK_OK)
        status = PHONEBOOK_IO_ERROR;
    return status;
}
