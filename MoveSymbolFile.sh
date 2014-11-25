#!/bin/sh
# Moves a .sym file to the right sub directory
symbol_file=$1

file_info=$(head -n1 $symbol_file)
IFS=' ' read -a splitlist <<< "${file_info}"
basefilename=${symbol_file:0:${#symbol_file} - 4}
dest_dir=$2/${splitlist[3]}
mkdir -p $dest_dir
mv $symbol_file $dest_dir
echo "$symbol_file -> $dest_dir/$symbol_file"

