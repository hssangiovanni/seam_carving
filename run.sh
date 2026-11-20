#!/bin/bash
g++ -o main src/main.cpp src/image.cpp

COLUMNS=${1:-200}
SOURCE=${2:-img/golfinho_2.jpg}
OUTPUT=${3:-output.jpg}

./main "$COLUMNS" "$SOURCE" "$OUTPUT"
