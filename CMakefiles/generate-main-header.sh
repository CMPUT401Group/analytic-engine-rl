#!/bin/bash

INCLUDE_DIR="$1"
OUTPUT_FILE="$2"
SUB_HEADERS_PREFIX="$3"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ -z "$INCLUDE_DIR" ]]; then
INCLUDE_DIR="./"  # Default to current directory.
fi

if [[ -z "$OUTPUT_FILE" ]]; then
OUTPUT_FILE="./"  # Default to current directory.
fi

# Remove so it doesn't list itself.
rm -rf "${OUTPUT_FILE}"

fileList=`ls "${INCLUDE_DIR}" | grep '\.h$'`
#echo $fileList

cat "${DIR}/license.h" >> "${OUTPUT_FILE}"
echo -e "\n#pragma once\n" >> "${OUTPUT_FILE}"
for file in $fileList
do
    echo "#include \"${SUB_HEADERS_PREFIX}/${file}\"" >> "${OUTPUT_FILE}"
done
