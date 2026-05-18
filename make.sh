#!/usr/bin/bash

echo "Making dssembly"
gcc ./src/*.c -o dssembly -Iinclude ./include/*.h -Wall -Werror && echo "Success!"
