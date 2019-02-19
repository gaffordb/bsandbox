#!/bin/bash

ROOT_DIR=../..
PROG=./test
echo "Running ${PROG} with no exec perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r .
echo "Running ${PROG} with exec perms, but no read perms... [should kill]"
$ROOT_DIR/sandbox $PROG -e

echo "Running test ${PROG} with perms to exec and read necessary files... [should allow]"

$ROOT_DIR/sandbox $PROG -e -r expl.txt

echo "Tests completed."
