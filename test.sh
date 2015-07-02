#!/bin/bash

if [ ! -d cereal-1.1.2 ]; then
    curl -o cereal-1.1.2.tar.gz -L https://github.com/USCiLab/cereal/archive/v1.1.2.tar.gz
    tar xf cereal-1.1.2.tar.gz
fi  

if [ ! -d dlib-18.16 ]; then  
    curl -o dlib-18.16.tar.gz -L https://github.com/davisking/dlib/archive/v18.16.tar.gz
    tar xf dlib-18.16.tar.gz
fi

if [ ! -d sqlite-amalgamation-3081002 ]; then  
    curl -O https://sqlite.org/2015/sqlite-amalgamation-3081002.zip
    unzip sqlite-amalgamation-3081002.zip
fi

CC=${CC:-clang}
CXX=${CXX:-clang++}

echo $CC
echo $CXX

$CC -c -O2 -I sqlite-amalgamation-3081002 -o sqlite.o sqlite-amalgamation-3081002/sqlite3.c
$CXX -c -O2 -std=c++11 -I include -I cereal-1.1.2/include -I dlib-18.16 -I sqlite-amalgamation-3081002 -o test.o test.cpp
$CXX -o test test.o sqlite.o

./test