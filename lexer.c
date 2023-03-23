// Name: Austin Lehman & Andrew Westlund
// Assignment: Program 1
// Class: CSC453 Fall 2022
// File: lexer.c

#include "global.h"

char lexbuf[BSIZE];
int lineno = 1;
int tokenval = NONE;

int lexan() {
	int t;
	int rc;

	while (1) {
   	//assign next character from stream
		t = getchar();

		//if t is white space eat the space
		if (t== ' ' || t == '\t') {
		} else if (t == '\n') {//if t is new line add to line number
			lineno = lineno + 1;
		//if t is a digit spit back out the digit and read in complete number and put into token val
		} else if (isdigit(t)) { 
			ungetc(t, stdin);
			scanf("%d", &tokenval);
	    	//check to see what Int type token is
			if (tokenval < 256) {
				rc = INT8;
			} else if (tokenval < 65536) {
				rc = INT16;
			} else {
				rc = INT32;
			}
			break;
		//check if t is letter or underscore
		} else if (isalpha(t) || t == '_') {
			int p, b = 0;
	    	//check to gather entire word stop at space or nonalphanumeric char
			while (isalnum(t) || t == '_') {
				lexbuf[b] = t;
				t = getchar();
				b = b + 1;
				//check to see that size of variable is not greater than buffersize
				if (b >= BSIZE) {
					error("compiler error");
				}
			}

			lexbuf[b] = EOS;
	   		//check if not end of file
			if (t != EOF) {
	    		//spit out most recently eaten value for later
				ungetc(t, stdin);
			}
	    	//check if buffer is in symbol table, and set to p. If not in table return -1
			p = lookup(lexbuf);

	    	//if not in symbol table add to symbol table
			if (p == -1) {
				//printf("** calling insert\n");
	    		//assign to symbol table and assign p to last entry with java index
				p = insert(lexbuf, ID, 0);
			}
	   		//token val is pos
			tokenval = p;
	    	//returning token type
			rc = symtable[p].token;

			break;
		} else if (t == EOF) {
			rc = DONE;
			break;
	} else {//return if error
		tokenval = NONE;
		rc = t;
		break;
	}
}

return(rc);
}
