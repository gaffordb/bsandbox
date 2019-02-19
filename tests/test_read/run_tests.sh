#!/bin/bash

ROOT_DIR=../..
PROG=./test
echo "Running ${PROG} ls with no perms... [should kill]"
$ROOT_DIR/sandbox $PROG
echo "Running ${PROG} with wrong perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r ../
echo "Running test ${PROG} with perms to read necessary files... [should allow]"
#jeesh louishe that's a lot of flags!
$ROOT_DIR/sandbox $PROG -r ./expl.txt
$ROOT_DIR/sandbox $PROG -r .

echo "Tests completed."
