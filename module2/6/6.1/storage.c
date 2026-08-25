#include "storage.h"

#include <stdio.h>

const char *storageStatusMessage(StorageStatus status)
{
    switch (status) {
    case STORAGE_OK:
        return "операция выполнена";
    case STORAGE_INVALID_ARGUMENT:
        return "некорректные параметры хранилища";
    case STORAGE_OPEN_ERROR:
        return "не удалось открыть файл";
    case STORAGE_READ_ERROR:
        return "ошибка чтения файла";
    case STORAGE_WRITE_ERROR:
        return "ошибка записи файла";
    case STORAGE_FORMAT_ERROR:
        return "некорректный формат файла";
    case STORAGE_DATA_ERROR:
        return "данные файла нельзя добавить в справочник";
    default:
        return "неизвестная ошибка хранилища";
    }
}

StorageStatus storageLoad(Phonebook *phonebook, const char *filename,
                          size_t *loaded_count)
{
    FILE *file;
    char line[256];
    size_t loaded = 0;
    StorageStatus status = STORAGE_OK;

    if (phonebook == NULL || filename == NULL)
        return STORAGE_INVALID_ARGUMENT;
    if (loaded_count != NULL)
        *loaded_count = 0;
    file = fopen(filename, "r");
    if (file == NULL)
        return STORAGE_OPEN_ERROR;

    while (fgets(line, sizeof(line), file) != NULL) {
        Contact contact = {0};
        PhonebookStatus phonebook_status;
        int consumed = 0;

        if (sscanf(line, "%d;%49[^;];%49[^;];%19[^\r\n]%n", &contact.id,
                   contact.name, contact.surname, contact.phone,
                   &consumed) != 4 ||
            (line[consumed] != '\0' && line[consumed] != '\n' &&
             !(line[consumed] == '\r' && line[consumed + 1] == '\n'))) {
            status = STORAGE_FORMAT_ERROR;
            break;
        }
        phonebook_status = phonebookInsert(phonebook, &contact);
        if (phonebook_status != PHONEBOOK_OK) {
            status = STORAGE_DATA_ERROR;
            break;
        }
        loaded++;
    }
    if (ferror(file))
        status = STORAGE_READ_ERROR;
    if (fclose(file) != 0 && status == STORAGE_OK)
        status = STORAGE_READ_ERROR;
    if (loaded_count != NULL)
        *loaded_count = loaded;
    return status;
}

StorageStatus storageSave(const Phonebook *phonebook, const char *filename)
{
    const ContactNode *node;
    FILE *file;
    StorageStatus status = STORAGE_OK;

    if (phonebook == NULL || filename == NULL)
        return STORAGE_INVALID_ARGUMENT;
    file = fopen(filename, "w");
    if (file == NULL)
        return STORAGE_OPEN_ERROR;

    for (node = phonebook->head; node != NULL; node = node->next) {
        if (fprintf(file, "%d;%s;%s;%s\n", node->contact.id,
                    node->contact.name, node->contact.surname,
                    node->contact.phone) < 0) {
            status = STORAGE_WRITE_ERROR;
            break;
        }
    }
    if (fclose(file) != 0 && status == STORAGE_OK)
        status = STORAGE_WRITE_ERROR;
    return status;
}
