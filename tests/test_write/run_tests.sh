#!/bin/bash

ROOT_DIR=../..
PROG=./test
echo "Running ${PROG} with no perms... [should kill]"
$ROOT_DIR/sandbox $PROG
echo "Running ${PROG} with wrong perms... [should kill]"
$ROOT_DIR/sandbox $PROG -w ../
$ROOT_DIR/sandbox $PROG -r .

echo "Running test ${PROG} with perms to read necessary files... [should allow]"
#jeesh louishe that's a lot of flags!
$ROOT_DIR/sandbox $PROG -w ./expl.txt
$ROOT_DIR/sandbox $PROG -w .

echo "Tests completed."
