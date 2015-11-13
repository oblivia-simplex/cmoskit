#! /bin/sh

sudo dd if=/dev/mem bs=1k skip=768 count=256 2> /dev/null | strings -n8 | grep AMIBIOS -A3
