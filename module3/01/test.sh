#!/bin/bash

PROGRAM="./copy"

echo "Сборка программы"

make clean
make

if [ $? -ne 0 ]; then
    echo "Ошибка сборки"
    exit 1
fi

echo
echo "------------------------------"
echo "Подготовка тестовых файлов"

echo "Hello world" > file1.txt
echo "Second test file" > file2.txt

# Пустой файл
touch empty.txt

# Файл больше BUF_SIZE
dd if=/dev/zero of=big.bin bs=1024 count=5 2>/dev/null

echo
echo "------------------------------"
echo "Тест 1: один файл"

$PROGRAM file1.txt

if cmp -s file1.txt file1.txt.copy; then
    echo "OK: file1.txt скопирован правильно"
else
    echo "ERROR: копия file1.txt отличается"
fi

echo
echo "------------------------------"
echo "Тест 2: несколько файлов"

$PROGRAM file1.txt file2.txt

if cmp -s file1.txt file1.txt.copy &&
   cmp -s file2.txt file2.txt.copy; then
    echo "OK: несколько файлов скопированы"
else
    echo "ERROR: ошибка копирования"
fi

echo
echo "------------------------------"
echo "Тест 3: отсутствующий файл"

$PROGRAM nofile.txt

echo
echo "------------------------------"
echo "Тест 4: существующий + отсутствующий"

$PROGRAM file1.txt nofile.txt file2.txt

if cmp -s file1.txt file1.txt.copy &&
   cmp -s file2.txt file2.txt.copy; then
    echo "OK: программа продолжила работу после ошибки"
else
    echo "ERROR: программа не обработала остальные файлы"
fi

echo
echo "------------------------------"
echo "Тест 5: пустой файл"

$PROGRAM empty.txt

if cmp -s empty.txt empty.txt.copy; then
    echo "OK: пустой файл скопирован"
else
    echo "ERROR: ошибка копирования пустого файла"
fi

echo
echo "------------------------------"
echo "Тест 6: большой файл"

$PROGRAM big.bin

if cmp -s big.bin big.bin.copy; then
    echo "OK: большой файл скопирован блоками"
else
    echo "ERROR: большой файл поврежден"
fi

echo
echo "------------------------------"
echo "Тест 7: FIFO"

$PROGRAM -p /tmp/testfifo file1.txt file2.txt

if cmp -s file1.txt file1.txt.copy &&
   cmp -s file2.txt file2.txt.copy; then
    echo "OK: FIFO работает"
else
    echo "ERROR: ошибка при работе с FIFO"
fi

echo
echo "------------------------------"
echo "Тест 8: запуск без аргументов"

$PROGRAM

echo
echo "------------------------------"
echo "Тест 9: -p без параметров"

$PROGRAM -p

echo
echo "Тесты завершены"