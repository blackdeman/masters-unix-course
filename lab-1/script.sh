#!/bin/bash

# создать sparse-файл с именем file512mb.dat
dd of=file512mb.dat bs=1 seek=512M count=0
# создать в файле файловую систему ext2
mke2fs file512mb.dat
# создание папки для монтирования
sudo mkdir -p /mnt/mountpoint-512mb
# подмонтировать файловую систему с использованием свободного loop device
sudo mount -o loop file512mb.dat /mnt/mountpoint-512mb
# пользователь deman становится владельцем папки /mnt/mountpoint-512mb
sudo chown deman /mnt/mountpoint-512mb
# создание простой файловой системы от имени пользователя deman
cd /mnt/mountpoint-512mb
mkdir -p dir1/dir2
mkdir dir3
echo "File content in dir2" > dir1/dir2/file-in-dir2.txt
echo "File content in dir3" > dir3/file-in-dir3.txt
echo "File content in upper level" > file-upper-level.txt