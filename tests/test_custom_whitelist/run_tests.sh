#!/bin/bash

ROOT_DIR=../..
PROG=pwd

echo "Running ${PROG} with no perms... [should kill]"
$ROOT_DIR/sandbox $PROG
echo "Running ${PROG} with wrong perms... [should kill]"
$ROOT_DIR/sandbox $PROG -p 80

echo "Running test ${PROG} with additional whitelist perm... [should allow]"
$ROOT_DIR/sandbox $PROG -p 79

echo "Tests completed."
