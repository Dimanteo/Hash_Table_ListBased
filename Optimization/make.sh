#!/bin/bash
gcc -g -pg -w -O0 -no-pie -c main.cpp -o main.o
gcc -g -pg -O0 -no-pie main.o hash.o -o gpaa.elf
