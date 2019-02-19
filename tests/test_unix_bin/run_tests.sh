#!/bin/bash

ROOT_DIR=../..
echo "Running test ls with no perms... [should kill]"
$ROOT_DIR/sandbox ls
echo "Running test ls with perms to read necessary files... [should allow]"

#jeesh louishe that's a lot of flags!
$ROOT_DIR/sandbox ls -r /home/walker/MyroC/lib -r /etc/ld.so.cache -r /lib/x86_64-linux-gnu/ -r /proc/filesystems -r /usr/lib/locale/locale-archive -r .

echo "Running test cat with just relevant libs... [should kill]"
$ROOT_DIR/sandbox cat success.txt -r /home/walker/MyroC/lib -r /etc/ld.so.cache -r /lib/x86_64-linux-gnu/ -r /proc/filesystems -r /usr/lib/locale/

echo "Running test cat with cwd perm and additional whitelist syscall... [should allow]"
$ROOT_DIR/sandbox cat success.txt -r /home/walker/MyroC/lib -r /etc/ld.so.cache -r /lib/x86_64-linux-gnu/ -r /proc/filesystems -r /usr/lib/locale/ -r . -p 221

echo "Tests completed."
