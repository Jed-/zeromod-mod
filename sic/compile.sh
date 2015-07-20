#!/bin/bash
g++ -c -o sic.o sic.c
g++ -c -o tools.o tools.cpp
g++ -o sic sic.o tools.o -lenet
