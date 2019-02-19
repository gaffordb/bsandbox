#!/bin/bash

ROOT_DIR=../..
PROG=./test
echo "Running ${PROG} with no fork perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r .
echo "Running ${PROG} with wrong perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r . -e
$ROOT_DIR/sandbox $PROG -f

echo "Running test ${PROG} with perms to read necessary files... [should allow]"
$ROOT_DIR/sandbox $PROG -r . -f 

PROG="./test 1"
echo "Running ${PROG} with no fork perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r .
echo "Running ${PROG} with wrong perms... [should kill]"
$ROOT_DIR/sandbox $PROG -r . -e

echo "Note that the child is not properly traced:"
$ROOT_DIR/sandbox $PROG -f

echo "Running test ${PROG} with perms to read necessary files... [should allow]"
$ROOT_DIR/sandbox $PROG -r . -f 


echo "Tests completed."
