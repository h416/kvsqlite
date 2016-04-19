#!/bin/bash

if [ ! -d cereal-1.1.2 ]; then
    curl -o cereal-1.1.2.tar.gz -L https://github.com/USCiLab/cereal/archive/v1.1.2.tar.gz
    tar xf cereal-1.1.2.tar.gz
fi

if [ ! -d dlib-18.18 ]; then
    curl -o dlib-18.18.tar.gz -L https://github.com/davisking/dlib/archive/v18.18.tar.gz
    tar xf dlib-18.18.tar.gz
fi

if [ ! -d sqlite-amalgamation-3120200 ]; then
    curl -O https://sqlite.org/2016/sqlite-amalgamation-3120200.zip
    unzip sqlite-amalgamation-3120200.zip
fi

CC=${CC:-clang}
CXX=${CXX:-clang++}

echo $CC
echo $CXX

echo "Environment: `uname -a`"
echo "Compiler: `$CXX --version`"


$CC -c -O2 -I sqlite-amalgamation-3120200 -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -o sqlite.o sqlite-amalgamation-3120200/sqlite3.c
$CXX -c -O2 -std=c++11 -I include -I cereal-1.1.2/include -I dlib-18.18 -I sqlite-amalgamation-3120200 -o test.o test.cpp
$CXX -o test test.o sqlite.o

./test
