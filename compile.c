// Austin Lehman & Andrew Westlund
// Assignment: Program 1
// Class: CSC453 Fall 2022
// File: compile.c

#include "global.h"
#include "javaclass.h"
#include "bytecode.h"

#define MAX_SIZE 127

struct ClassFile cf;
int index1, index2, index3;
int label1, label2;

int tk; // lookahead token
int stackDepth;

int retLoc[2*MAX_SIZE];
int numRets = 0;

extern int assign_var_index(int tokenval);
extern int match(int token); 
extern void stmt();
extern void opt_stmts();
extern void expr();
extern void term();
extern void moreterms();
extern void factor();
extern void morefactors();

//*******************************************************************************

void
java_preamble() {
	// set up new class file structure
	init_ClassFile(&cf);

	// class has public access
	cf.access = ACC_PUBLIC;

	// class name is "Calc"
	cf.name = "Calc";

	// no fields
	cf.field_count = 0;

	// one method
	cf.method_count = 1;

	// allocate array of methods (just one "main")
	cf.methods = (struct MethodInfo*)malloc(cf.method_count * sizeof(struct MethodInfo));

	// method has public access and is static
	cf.methods[0].access = ACC_PUBLIC | ACC_STATIC;

	// method name is "main"
	cf.methods[0].name = "main";

	// Should we change this? Since there are returns?
	// method descriptor of "void main(String[] arg)"
	cf.methods[0].descriptor = "([Ljava/lang/String;)V";

	// max operand stack size of this method
	cf.methods[0].max_stack = MAX_SIZE;

	// the number of local variables in the local variable array
	//   local variable 0 contains "arg"
	//   local variable 1 contains "val"
	//   local variable 2 contains "i" and "result"
	cf.methods[0].max_locals = 3 + MAX_SIZE;

	// set up new bytecode buffer
	init_code();

	// generate code
	/*LOC*/
	/*000*/	emit(aload_0);			// loads arg ref to operand (op) stack
	/*001*/	emit(arraylength);		// pops arg ref and pushes arg.length
	/*002*/	emit2(newarray, T_INT);		// makes a new int[arg.length] array ref
	/*004*/	emit(astore_1);			// val = new int[arg.length]
	/*005*/	emit(iconst_0);			// pushes 0 on op stack
	/*006*/	emit(istore_2);			// i = 0
	label1 = pc;			// label1:
	/*007*/	emit(iload_2);			// pushes i to op stack
	/*008*/	emit(aload_0);			// pushes arg ref to stack
	/*009*/	emit(arraylength);		// pops arg ref and pushes arg.length
	label2 = pc;			
	/*010*/	emit3(if_icmpge, PAD);		// if i >= arg.length then goto label2
	/*013*/	emit(aload_1);			// push argCopy to op
	/*014*/	emit(iload_2);			// push i to op
	/*015*/	emit(aload_0);			// push arg to op
	/*016*/	emit(iload_2);			// push i to op
	/*017*/	emit(aaload);			// push arg[i] parameter for parseInt
	index1 = constant_pool_add_Methodref(&cf, "java/lang/Integer", "parseInt", "(Ljava/lang/String;)I");
	/*018*/	emit3(invokestatic, index1);	// invoke Integer.parseInt(arg[i])
	/*021*/	emit(iastore);			// val[i] = Integer.parseInt(arg[i])
	/*022*/	emit32(iinc, 2, 1);		// i++
	/*025*/	emit3(goto_, label1 - pc);	// goto label1
	backpatch(label2, pc - label2);	// label2:
}

//*******************************************************************************

void
java_postamble() {

	for (int i = 0; i < numRets; i++) {		// for every "return"
		backpatch(retLoc[i], pc - retLoc[i]);	// backpatch the offset to get "here"
	}

	// Print out whatever we return
	index2 = constant_pool_add_Fieldref(&cf, "java/lang/System", "out", "Ljava/io/PrintStream;");
	/*036*/	emit3(getstatic, index2);	// get static field System.out of type PrintStream
	/*039*/	emit(iload_2);			// push parameter for println()
	index3 = constant_pool_add_Methodref(&cf, "java/io/PrintStream", "println", "(I)V");
	/*040*/	emit3(invokevirtual, index3);	// invoke System.out.println(result)
	/*043*/	emit(return_);			// return

	// length of bytecode is in the emmiter's pc variable
	cf.methods[0].code_length = pc;

	// must copy code to make it persistent
	cf.methods[0].code = copy_code();

	// save class file to "Comp.class"
	save_classFile(&cf);
}

//*******************************************************************************

int
	match(int token) {
		int rc = 0;  // in case we don't have a match, we return "false"

	if (token == tk) {
		rc = 1;
	tk = lexan(); // get next token
	}

	return(rc);
}

//*******************************************************************************

void
stmt() {
	int var_index;
	int loc = -1;
	int if_loc = -1;
	int else_loc = -1;

	switch (tk) {
	// stmt → ’{’ opt stmts ’}’
		case LEFT_CURLY:
			match(LEFT_CURLY);
			opt_stmts();
			if (!match(RIGHT_CURLY)) {
				error("Expected closing curly brace");
			}
			break;

		case ID: // OR stmt -> ID = expr ;
			var_index = symtable[tokenval].var_index;		// get id's var_index
			if (var_index < 0) {
				error("Attempting to use keyword as variable");
			} else if (var_index == 0) {
			var_index = assign_var_index(tokenval); 	// if id didn't have a real var_index yet, initialize it
			}
			// else case already taken care of, just overwrite existing variable

			match(ID);
			if (!match(EQ)) {
				error("Expected assignment operator");
			}
			expr();

			if (stackDepth >= 1) {		// if there is anything on op stack
				emit2(istore, var_index);	// pop the op stack and put it in the id's index in the locals table
				stackDepth--;
			} else {
				error("No rvalue for assignment operator");
			}

			if (!match(';')) {
				error("Expected ';'");
			}
			break;

		case IF:
			// OR stmt -> if ’(’ expr ’)’ stmt else stmt
			match(IF);
			if (!match('(')) {
				error("Mising parenthesis");
			}
			expr();				// pushes expression in () onto the op stack

			if (!match(')')) {
				error("Mising parenthesis");
			}

			emit(iconst_0);			// push 0 onto op stack
			if_loc = pc;			// IF_LOC:
			emit3(if_icmpeq, 0);		// if expression is false go to A

			// Do this if expression is true
			stmt();

			else_loc = pc; 			// ELSE_LOC
			emit3(goto_,0);			// go to B if expr was true

			if(!match(ELSE)){ //match else statement
				error("Missing Else Statement");
			}

			backpatch(if_loc, pc-if_loc);	// I AM A (If expr is false, skip to here)

			stmt(); 				// do this statement if expr false

			backpatch(else_loc,pc-else_loc);	//I AM B

			break;

		case WHILE:
			// stmt -> while ’(’ expr ’)’ stmt
			match(WHILE);
			if (!match('(')) {
				error("Mising parenthesis");
			}

			int test_loc = pc;		// TEST_LOC:
			expr();

			if (!match(')')) {
				error("Mising parenthesis");
			}

			emit(iconst_0);		// push 0 to op stack

			loc = pc;			// LOC:
			emit3(if_icmpeq, 0);	// if expr was false, skip to D

			stmt();

			emit3(goto_, test_loc-pc);	// Go back to TEST_LOC
			backpatch(loc, pc-loc);	// I AM D
			break;

		case RET:
			// OR stmt -> return expr ;
			match(RET);
			expr();

			if (stackDepth >= 1) {		// makes sure something is being returned
				emit(istore_2);			// pop op stack (value will be from expr()) and store it on locals[2], the result value
				retLoc[numRets] = pc;		// store the locations of the returns, so postamble can give them the right offsets
				emit3(goto_, retLoc[numRets]);	// go to preamble
				numRets++;
			} else {
				error("No value to return, stack empty");
			}

			//match for expected ;
			if (!match(';')) {
				error("Expected ';'");
			}

			break;

		default:
			error("Unexpected token, expected statement");
			break;
			}
}

//*******************************************************************************

// opt_stmts -> stmt opt_stmts | ε
void
opt_stmts() {
	// opt_stmts -> ε
	if(!(tk == '}')) {	// if it has reached a closing brace, stop recursing
		// OR opt_stmts -> stmt opt_stmts
		//call stmt and or opt_stmts
		stmt();
		opt_stmts();
	}
}

//*******************************************************************************

// expr -> term moreterms
void
expr() {	// if it has reached a closing paren or semicolon, stop recursing
	if(!(tk == ')'|| tk == ';')) {
		//call term and or moreterms
		term();
		moreterms();
	}
}

//*******************************************************************************

// term -> factor morefactors
void
term() {
//check if we have recieved an ε to stop calling factor/morefactors
	if(!(tk == ')'|| tk == ';')) {
		//call factor and or more factors
		factor();
		morefactors();
	}
}

//*******************************************************************************

// moreterms -> + term moreterms | - term moreterms | ε
void
moreterms() {
	// moreterms -> ε, if we reach a ; or ) we stop recursing
	if (!(tk== ';' || tk==')')) {
		switch (tk) {
    		// OR moreterms -> + term moreterms
			case PLUS:
				match(PLUS);
				term();
				if (stackDepth < 2) {
					error("Not enough operands for '+'");
				}
				emit(iadd);	// pop the top two values of the stack and push the sum
				stackDepth--;
				moreterms();	// check for more terms
				break;

		    // OR moreterms -> + term moreterms
			case MINUS:
				match(MINUS);//match minus
				term();//call term
				if (stackDepth < 2) {//check if can subtract
					error("Not enough operands for '-'");
				}
				emit(isub);	// pop the top two values of the stack and push the difference (second_to_last_value - last_value)
				stackDepth--; //subtract stackdepth to account for two removal and one add
				moreterms(); //call moreterms 
				break;

			default:
				error("Expected '+' or '-'");
		}
	}
}

//*******************************************************************************

// morefactors -> * factor morefactors | factor morefactors | % factor morefactors | ε
void
morefactors() {
// morefactors -> ε
	if (!(tk=='+' || tk=='-' || tk==';' || tk==')')) {
		switch (tk) {
    		// OR morefactors -> * factor morefactors
			case MUL:
				match(MUL); //match mul
				factor();
				if(stackDepth < 2){ //check for enough to operate on
					error("Not enough operands for '*'");
				}
				emit(imul);		// pop the last two values of the op stack and push their product
				stackDepth--; //change depth to account for removal of 2 and addition of one 
				morefactors(); 
				break;

	    	// OR morefactors -> / factor morefactors
			case DIV:
				match(DIV);
				factor();
				if(stackDepth < 2){ //check for enough to operate on 
					error("Not enough operands for '/'");
				}
		    	emit(idiv);		// pop the top two values of the stack and push the quotient (second_to_last_value / last_value)
		    	stackDepth--;   //change depth to account for removal of 2 and addition of one
		    	morefactors();
		    	break;

	   	 	// OR morefactors -> % factor morefactors
	    	case MOD:
		    	match(MOD);
		    	factor();
		    	if(stackDepth < 2){
		    		error("Not enough operands for '%'");
		    	}
				emit(irem);		// pop the top two values of the stack and push the mod(remainder) (second_to_last_value % last_value)
				stackDepth--;   //change depth to account for removal of 2 and addition of one
				morefactors();
				break;

			default:
				error("Expected '*', '/' or '%'");
		}
	}
}

//*******************************************************************************

// factor -> ’(’ expr ’)’ | - factor | NUM | ID | arg ’[’ INT8 ’]’
void
factor() {
	int var_index = -1;

	switch (tk) {
    	// factor -> ’(’ expr ’)’
		case LEFT_PAREN:
	    	// Just push the expression to the top of the stack
			match('(');
			expr();
			if (!match(')')) {
				error("Missing close parenthesis");
			}
			break;

		case MINUS:
		    // factor -> - factor
			match('-');
			if (stackDepth < 1) {
				error("Not enough operands for negation");
			}
			factor();
		    emit(ineg);		// pop the op stack and push the product of it and -1
		    break;

		// factor -> NUM
	    case INT8:
		    // push the byte value onto the stack
		    emit2(bipush, tokenval);
		    stackDepth++;
		    match(INT8);
		    break;

	    case INT16:
		    // push the short value as 2 bytes onto the stack
		    emit3(sipush, tokenval);
		    stackDepth++;
		    match(INT16);
		    break;

	    case INT32:
		    // add the integer value to the constant_pool and push a handle to it?
		    emit2(ldc, constant_pool_add_Integer(&cf, tokenval));
		    stackDepth++;
		    match(INT32);
		    break;

		// factor -> ID
	    case ID:
	    	var_index = symtable[tokenval].var_index;		// get the id's var_index
	    	if (var_index < 0) {
				error("Keywords are not valid lvalues");	// if it's less than 0, it shouldn't be here
			} else if (var_index == 0) {
				error("Variable does not exist");		// if it is 0, it hasn't been declared yet and has no value
			} else {
		
	    	emit2(iload,var_index);				// if the id already exists, push locals[var_index] onto the stack
	    	stackDepth++;
			}
			match(ID);
			break;

	
		// factor -> arg ’[’ INT8 ’]’
		case ARG:
			match(ARG);
			//check for opening bracket 
			if (!match('[')) {
				error("Mising bracket");
			}

		    // after the last match, we changed tokenval to be the number in the scanf("%d", &tokenval); in lexer.c
		    emit(aload_1);		// push the argCopy array with the parsed ints
		    emit2(bipush,tokenval);	// push the index of arg that we want
		    emit(iaload);		// pops the index and arrayCopy ref, then pushes argCopy[tokenval]
		    stackDepth++;
		    //check that int8 has been passed
		    if(!match(INT8)) {
		    	error("Missing INT8");
		    }
		    if (!match(']')) {//check for required closing bracket
		    	error("Mising bracket");
		    }

		    break;

		default:
			error("Expected a factor");
		   	break;
	}
}

//*******************************************************************************

int
main() {
	init();		// initializes the symbol table

	java_preamble();	// puts the argCopy with the int values in locals[1]

	tk = lexan();	// gets the first token

	if (!(tk == DONE || tk == EOF)) {
		stmt();		// starting nonterminal 
	}

	java_postamble();	// backpatches the return goto_'s so they skip to the end of the code and print the result

	// Useful to do the following when debugging
	//dumpSumbolTable();

	return 0;
}
