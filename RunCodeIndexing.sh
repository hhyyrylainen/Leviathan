#!/bin/sh
gtags
cscope -b -e -R -U -q
ctags -R -e
