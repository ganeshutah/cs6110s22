#!/bin/bash
rumur german.m --output germanout.c
cc -march=native -std=c11 -O3 germanout.c -lpthread
./a.out
