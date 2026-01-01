#!/bin/bash

mkdir -p obj
mkdir -p bin

declare -a all_object_files
declare -a engine_files other_files

categorize_object_file() {
    local file="$1"

    case "$file" in
        obj/engine/*)
            engine_files+=("$file")
            ;;
        *)
            other_files+=("$file")
            ;;
    esac
}

# Recursive compilation function
list_files_recursive() {
    local dir="$1"
    for file in "$dir"/*; do
        if [[ -f "$file" && "$file" == *.c ]]; then
            local base="${file#./src/}"
            base="${base%.c}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")

            echo "Compiling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"

            gcc -I./src -c "$file" -o "$output_file" -Wall -Wextra -s -fno-ident -fno-asynchronous-unwind-tables -g0
            categorize_object_file "$output_file"
        elif [[ -f "$file" && "$file" == *.s ]]; then
            local base="${file#./src/}"
            base="${base%.s}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")

            echo "Assembling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"
            clang -c "$file" -o "$output_file"
            categorize_object_file "$output_file"
        elif [ -d "$file" ]; then
            list_files_recursive "$file"
        fi
    done
}

mkdir -p obj
echo "Compiling source files..."
list_files_recursive "./src"

# Print categorized files
echo ""
echo "=== Categorized Object Files ==="
echo "Engine files: ${engine_files[@]}"
echo "Other files: ${other_files[@]}"

# Build the link command
echo ""
echo "=== Linking ==="
cc -o "bin/main" \
    "${engine_files[@]}" \
    "${other_files[@]}" -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
if [ $? -eq 0 ]; then
    echo "Linking successful: bin/main"
    cd bin
    ./main
    cd ..
else
    echo "Linking failed!"
fi
