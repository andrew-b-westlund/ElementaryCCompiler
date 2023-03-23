// Name: Austin Lehman & Andrew Westlund
// Assignment: Program 1
// Class: CSC453 Fall 2022
// File: lextest.c

#include <stdio.h>
#include "global.h"

int
main(int argc, char* argv[])
{	//calling init function to initialize the symbol table with keywords
	init();

	fprintf(stderr, "argc = %d\nargv[0] = %s\n", argc, argv[0]);

	int t = 0;
    do {//set t to next 
    	t=lexan();
    	if (t != NONE) {
    		putchar('<');
    		if (t < DONE) {
    			printf("%c,%d> ", (char)t, -1);
    		} else {
    			switch(t) {
    				case DONE:
    				printf("DONE");
    				printf(",%d> ", symtable[tokenval].var_index);
    				break;

		    		//case if ID
    				case ID:
    				printf("ID");
    				printf(",%d> ", symtable[tokenval].var_index);
    				break;



					//PRINT KEYWORDS
    				case IF:
    				printf("IF");
    				printf(",%d> ",symtable[tokenval].var_index);
    				break;


    				case ELSE:
    				printf("ELSE");
    				printf(",%d> ",symtable[tokenval].var_index);
    				break;


    				case WHILE:
    				printf("WHILE");
    				printf(",%d> ",symtable[tokenval].var_index);
    				break;

    				case RET:
    				printf("RET");
    				printf(",%d> ",symtable[tokenval].var_index);
    				break;


    				case ARG:
    				printf("ARG");
    				printf(",%d> ",symtable[tokenval].var_index);
    				break;




			//PRINT NUMBERS
    				case INT8:
    				printf("INT8");
    				printf(",%d> ", tokenval);
    				break;

    				case INT16:
    				printf("INT16");
    				printf(",%d> ", tokenval);
    				break;

    				case INT32:
    				printf("INT32");
    				printf(",%d> ", tokenval);
    				break;
    			}
    		}
    	}
    } while (!(t == DONE || t == EOF));
    putchar('\n');
    //dumpSumbolTable();
    return 0;



    
}
