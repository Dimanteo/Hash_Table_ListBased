#!/bin/bash
valgrind --tool=callgrind --collect-atstart=no ./$1
