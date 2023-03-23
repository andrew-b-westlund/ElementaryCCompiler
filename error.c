// Name: Austin Lehman & Andrew Westlund
// Assignment: Program 1
// Class: CSC453 Fall 2022
// File: error.c

/**** error.c ****/

#include "global.h"

void error(char *m) {
    fprintf(stderr, "line %d: %s\n", lineno, m);
    exit(1);
}
