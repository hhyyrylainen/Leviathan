#!/bin/sh
# Dumps symbols of a target and then moves them

# This should be executed in the Symbols directory

filename=$(basename "$1")

# Run the symbol dumper
./dump_syms $1 > "$filename.sym"

echo "Created symbol file $1.sym"

# Move it to the right place
sh MoveSymbolFile.sh "$filename.sym" "$filename"

exit 0
