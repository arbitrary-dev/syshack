#!/bin/sh

clang-format -i src/* include/*
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug ..
mv compile_commands.json ../
make
