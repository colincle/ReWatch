#!/bin/bash

# Ensure AStyle is installed
command -v astyle >/dev/null 2>&1 || { echo "AStyle is not installed. Aborting."; exit 1; }

# Default directory is the current directory
DIR=${1:-.}

# Create the 'orig' folder if it doesn't exist
mkdir -p "$DIR/orig"

# Find all .cpp and .hpp files recursively and apply AStyle formatting
find "$DIR" -type f \( -iname \*.cpp -o -iname \*.hpp \) -exec astyle --style=allman --indent=tab \
--break-blocks \
--pad-oper \
--keep-one-line-statements \
--keep-one-line-blocks \
--pad-include \
--unpad-paren \
--squeeze-lines=1 \
--squeeze-ws \
--break-closing-braces \
--break-elseifs \
--break-one-line-headers \
--add-braces {} \;

# Move all .orig files to the 'orig' folder
find "$DIR" -type f -iname "*.orig" -exec mv {} "$DIR/orig" \;

echo "Formatting completed and backup files moved to the 'orig' folder."
