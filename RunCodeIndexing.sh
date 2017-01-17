#!/bin/sh
#cscope -b -q
find . -iname "*.h" -o -iname "*.cpp" -not -path "./build/*" > cscope.files
cscope -b

