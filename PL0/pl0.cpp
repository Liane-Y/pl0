// pl0 compiler source code

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>

#include "pl0.h"
#include "set.h"
#include <iostream>

//for debug
int debug_1 = 0;


//////////////////////////////////////////////////////////////////////
// print error message.
symset phi, declbegsys, statbegsys, facbegsys, relset;
//for break
int cx_for_break = 0;
//for exit
int cx_for_exit = 0;
//input and output
void printvar();
void inputvar();

void error(int n) {
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

  //////////////////////////////////////////////////////////////////////
void getch(void) {

	// 读取的字符保存在ch中
	if (cc == ll) {
		if (feof(infile)) {
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n')) {

			if (ch == '/') {//newly
				line[++ll] = ch;
				if ((!feof(infile))
					&& ((ch = getc(infile)) != '\n')) {
					if (ch == '/') {
						line[ll] = ' ';
						while ((!feof(infile))
							&& ((ch = getc(infile)) != '\n')) {
						}
					}
					else if (ch == '*') {
						line[ll] = ' ';
						while (!feof(infile)) {
							ch = getc(infile);
							if (ch == '*'
								&& (!feof(infile))
								&& ((ch = getc(infile)) == '/')) {
								break;
							}
						}
					}
					else {
						printf("%c", ch);
						line[++ll] = ch;
					}
				}
				else {
					break;
				}
			}
			else {

				printf("%c", ch);
				line[++ll] = ch;
			}
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
  // 状态被限定在全局变量中
  // sym指示了当前symbol的类型，如果是标识符，其值显示指定
void getsym(void) {
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();

	if (isalpha(ch)) { // symbol is a reserved word or an identifier.
		k = 0;
		do {
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;//word[0]被用来保存现在的id，占位
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER; // symbol is an identifier
	}
	else if (ch == '[') {
		sym = SYM_LSQUARE;
		getch();
	}
	else if (ch == ']') {
		sym = SYM_RSQUARE;
		getch();
	}
	else if (isdigit(ch)) { // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do {
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25); // The number is too great.
	}
	else if (ch == ':') {
		getch();
		if (ch == '=') {
			sym = SYM_BECOMES; // :=
			getch();
		}
		else {
			sym = SYM_NULL; // illegal?
		}
	}
	else if (ch == '>') {
		getch();
		if (ch == '=') {
			sym = SYM_GEQ; // >=
			getch();
		}
		else {
			sym = SYM_GTR; // >
		}
	}
	else if (ch == '<') {
		getch();
		if (ch == '=') {
			sym = SYM_LEQ; // <=
			getch();
		}
		else if (ch == '>') {
			sym = SYM_NEQ; // <>
			getch();
		}
		else {
			sym = SYM_LES; // <
		}
	}
	else if (ch == '&') {
		getch();
		if (ch == '&') {
			sym = SYM_AND;
			getch();
		}
	}
	else if (ch == '|') {
		getch();
		if (ch == '|') {
			sym = SYM_OR;
			getch();
		}
	}
	else if (ch == '!') {
		sym = SYM_NOT;
		getch();
	}
	else { // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i) {
			sym = ssym[i]; //运算符
			getch();
		}
		else {
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

  //////////////////////////////////////////////////////////////////////
  // generates (assembles) an instruction.
void gen(int x, int y, int z) {
	if (cx > CXMAX) {
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

  //////////////////////////////////////////////////////////////////////
  // tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n) {
	symset s;

	if (!inset(sym, s1)) {
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

  //////////////////////////////////////////////////////////////////////
int dx; // data allocation index

		// enter object(constant, variable or procedre) into table.
void enter(int kind, int len = 1) {
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind) {
	case ID_CONSTANT:
		if (num > MAXADDRESS) {
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*)&table[tx];
		mk->level = level;
		mk->address = dx + len;
		dx += len;
		break;
	case ID_PROCEDURE:
		mk = (mask*)&table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

  //////////////////////////////////////////////////////////////////////
  // locates identifier in symbol table.
  // 结果以下标的形式给出,如果返回0,代表符号不存在
int position(char* id) {
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

  //////////////////////////////////////////////////////////////////////
void constdeclaration() {
	if (sym == SYM_IDENTIFIER) {
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES) {
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER) {
				enter(ID_CONSTANT);
				getsym();
			}
			else {
				error(2); // There must be a number to follow '='.
			}
		}
		else {
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration


void dimDeclaration(void) {
	if (sym == SYM_LSQUARE) {
		getsym();
		int ks;
		if (sym == SYM_NUMBER) {
			ks = num;
			getsym();
			if (sym == SYM_RSQUARE) {
				getsym();
				dimDeclaration();
				base_dim *= ks;
			}
			else {
				error(1);
			}
		}
	}
	else if (sym == SYM_COMMA || sym == SYM_SEMICOLON) {
		base_dim = 1;
	}
	else {
		error(1);
	}
}
//////////////////////////////////////////////////////////////////////
void vardeclaration(void) {
	//todo:添加数组声明
	if (sym == SYM_IDENTIFIER) {
		getsym();
		dimDeclaration();
		enter(ID_VARIABLE, 1);
		//		getsym();
	}
	else {
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration


  //////////////////////////////////////////////////////////////////////
void listcode(int from, int to) {
	int i;

	printf("\n");
	for (i = from; i < to; i++) {
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

  //////////////////////////////////////////////////////////////////////
void factor(symset fsys) {
	void expression(symset fsys);
	void expr_condition(symset fsys);
	int i;
	symset set;
	bool flag = false;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	while (inset(sym, facbegsys)) {
		if (sym == SYM_NOT) {
			flag = true;
			getsym();
			continue;
		}
		else if (sym == SYM_IDENTIFIER) {
			if ((i = position(id)) == 0) {
				error(11); // Undeclared identifier.
			}
			else {
				switch (table[i].kind) {
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			if (flag) {
				gen(OPR, 0, OPR_NOT);
				flag = false;
			}
			getsym();
		}
		else if (sym == SYM_NUMBER) {
			if (num > MAXADDRESS) {
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			if (flag) {
				gen(OPR, 0, OPR_NOT);
				flag = false;
			}
			getsym();
		}
		else if (sym == SYM_LPAREN) {
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expr_condition(set);
			destroyset(set);
			if (sym == SYM_RPAREN) {
				getsym();
				if (flag) {
					gen(OPR, 0, OPR_NOT);
					flag = false;
				}
			}
			else {
				error(22); // Missing ')'.
			}
		}
		test(fsys, createset(SYM_LPAREN, SYM_NOT, SYM_NULL), 23);
	} // while
} // factor

  //////////////////////////////////////////////////////////////////////
void term(symset fsys) {
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH) {
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES) {
			gen(OPR, 0, OPR_MUL);
		}
		else {
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

  //////////////////////////////////////////////////////////////////////
void expression(symset fsys) {
	int addop;
	symset set;



	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	if (sym == SYM_PLUS || sym == SYM_MINUS) {
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_MINUS) {
			gen(OPR, 0, OPR_NEG);
		}
	}
	else {
		term(set);
	}

	while (sym == SYM_PLUS || sym == SYM_MINUS) {
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS) {
			gen(OPR, 0, OPR_ADD);
		}
		else {
			gen(OPR, 0, OPR_MIN);
		}
	} // while


	destroyset(set);
} // expression


  //关系表达式
void condition(symset fsys) {
	int relop;
	symset set;

	if (sym == SYM_ODD) {
		getsym();
		expression(fsys);
		gen(OPR, 0, OPR_ODD);
	}
	else {
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (inset(sym, relset)) {
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop) {
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition
  //////////////////////////////////////////////////////////////////////
void expr_condition(symset fsys) {
	int andop;
	symset set;
	int cx_jpc, cx_jmp;
	set = uniteset(fsys, createset(SYM_AND, SYM_OR, SYM_NULL));
	if (sym == SYM_AND || sym == SYM_OR) {
		andop = sym;
		getsym();
		if (andop == SYM_AND) {
			cx_jpc = cx;
			gen(JPC, 0, 0);
			gen(LIT, 0, 1);
			//			expression(set);
			condition(set);
			gen(OPR, 0, OPR_AND);
			cx_jmp = cx;
			gen(JMP, 0, 0);
			code[cx_jpc].a = cx;
			gen(LIT, 0, 0);
			code[cx_jmp].a = cx;
		}
		else {
			cx_jpc = cx;
			gen(JC, 0, 0);
			gen(LIT, 0, 0);
			//			expression(set);
			condition(set);
			gen(OPR, 0, OPR_OR);
			cx_jmp = cx;
			gen(JMP, 0, 0);
			code[cx_jpc].a = cx;
			gen(LIT, 0, 1);
			code[cx_jmp].a = cx;
		}
		//		expression(set);
	}
	else {
		//			expression(set);
		condition(set);
	}


	while (sym == SYM_AND || sym == SYM_OR) {
		andop = sym;
		getsym();
		if (andop == SYM_AND) {
			cx_jpc = cx;
			gen(JPC, 0, 0);
			gen(LIT, 0, 1);
			//			expression(set);
			condition(set);
			gen(OPR, 0, OPR_AND);
			cx_jmp = cx;
			gen(JMP, 0, 0);
			code[cx_jpc].a = cx;
			gen(LIT, 0, 0);
			code[cx_jmp].a = cx;
		}
		else {
			cx_jpc = cx;
			gen(JC, 0, 0);
			gen(LIT, 0, 0);
			//			expression(set);
			condition(set);
			gen(OPR, 0, OPR_OR);
			cx_jmp = cx;
			gen(JMP, 0, 0);
			code[cx_jpc].a = cx;
			gen(LIT, 0, 1);
			code[cx_jmp].a = cx;
		}
	} // while

	destroyset(set);
}


//////////////////////////////////////////////////////////////////////
//语句.
//一个语句为以下一种:赋值语句,条件判断,循环或者begin end包含的语句块
void statement(symset fsys) {
	int i, cx1, cx2;
	symset set1, set;
	//int cx_for1;
	int cx_for2, cx_for3;
	int cx_jmp1, cx_jmp2;


	if (sym == SYM_EXIT) {
		getsym();
		cx_for_exit = cx;

		gen(JMP, 0, 0);

	}
	if (sym == SYM_BREAK) {
		getsym();
		//cx_for_break=cx_for_break+2;
		cx_for_break = cx;
		gen(JMP, 0, 0);

	}


	if (sym == SYM_IDENTIFIER) { // variable assignment
								 //赋值语句的右值应该是一个可计算值的表达式
		mask* mk;
		if (!(i = position(id))) {
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE) {
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES) {
			getsym();
		}
		else {
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask*)&table[i];
		if (i) {
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_CALL) { // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER) {
			error(14); // There must be an identifier to follow the 'call'.
		}
		else {
			if (!(i = position(id))) {
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE) {
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else {
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	}


	else if (sym == SYM_IF) { // if statement

		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		//		condition(set);
		expr_condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN) {
			getsym();
		}
		else {
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		int cx_jmp = cx;
		gen(JMP, 0, 0);
		if (sym == SYM_SEMICOLON) {
			getsym();
		}
		code[cx1].a = cx;

		if (sym == SYM_ELSE) {
			getsym();
			statement(fsys);
			code[cx_jmp].a = cx;
		}


	}
	else if (sym == SYM_BEGIN) { // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)) {
			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END) {
			getsym();
		}
		else {
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE) { // while statement

		cx1 = cx;//CX1保存了while条件句位置
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		expr_condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);//CX2保存了JPC地址
		if (sym == SYM_DO) {
			getsym();
		}
		else {
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		printf("%d2333333", cx);

		code[cx2].a = cx;
		//judge if break
		if (code[cx_for_break].a == 0) {
			code[cx_for_break].a = cx;
		}
		//judge if exit
		if (code[cx_for_exit].a == 0) {
			code[cx_for_exit].a = cx;
		}
	}

	else if (sym == SYM_FOR) {
		getsym();
		symset set_go = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset(set_go, fsys);
		statement(set);
		if (sym == SYM_SEMICOLON) {
			getsym();
		}
		cx_for2 = cx;
		expr_condition(set);
		cx_for3 = cx;
		gen(JPC, 0, 0);
		cx_jmp1 = cx;
		gen(JMP, 0, 0);
		if (sym == SYM_SEMICOLON) {
			getsym();
		}
		set = uniteset(createset(SYM_DO, SYM_NULL), fsys);
		cx_jmp2 = cx;
		statement(set);
		gen(JMP, 0, cx_for2);
		code[cx_jmp1].a = cx;
		if (sym == SYM_DO) {
			getsym();
		}
		statement(fsys);
		gen(JMP, 0, cx_jmp2);
		//judge if break
		if (code[cx_for_break].a == 0) {
			code[cx_for_break].a = cx;
		}
		code[cx_for3].a = cx;

		//judge if exit
		if (code[cx_for_exit].a == 0) {
			code[cx_for_exit].a = cx;
		}

	}
	//printf
	//PRINT A,B,E
	/* if (sym == SYM_PRINTF) { // constant declarations
	getsym();
	do {
	printvar();
	while (sym == SYM_COMMA) {
	getsym();
	printvar();
	}
	if (sym == SYM_SEMICOLON) {
	//getsym();

	}
	else {
	error(5); // Missing ',' or ';'.
	}
	} while (sym == SYM_IDENTIFIER);
	} */
	if (sym == SYM_PRINTF) {

		symset ttt = uniteset(fsys, createset(SYM_COMMA, SYM_SEMICOLON, SYM_NULL));

		do {
			getsym();
			expr_condition(ttt);
			gen(PT, 0, 0);
		} while (sym == SYM_COMMA);

		if (sym == SYM_SEMICOLON) {
			getsym();
		}
	}
	//INPUT A,B,C
	if (sym == SYM_INPUT) { // constant declarations
		getsym();
		do {
			inputvar();
			while (sym == SYM_COMMA) {
				getsym();
				inputvar();
			}
			if (sym == SYM_SEMICOLON) {
				//getsym();
			}
			else {
				error(5); // Missing ',' or ';'.
			}
		} while (sym == SYM_IDENTIFIER);
	}
	//judge if exit
	if (code[cx_for_exit].a == 0) {
		code[cx_for_exit].a = cx;
	}

	//printf("cx:%d  \n",cx);

	test(fsys, phi, 19);

	//todo:for循环实现
} // statement

  //////////////////////////////////////////////////////////////////////

  //block处理程序体，以const,var,procedure和语句(statement)作为开始符号
  //其中const,procedure的声明是递归的,而procedures-> procedure ident ; 程序体 ; 
  //故procedure中还会调用block,这些定义会循环到不出现这些符号
  //之后进行语句的处理,程序最后结束
void block(symset fsys) {
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*)&table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL) {
		error(32); // There are too many levels.
	}
	do {
		if (sym == SYM_CONST) { // constant declarations
			getsym();
			do {
				constdeclaration();
				while (sym == SYM_COMMA) {
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON) {
					getsym();
				}
				else {
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR) { // variable declarations
			getsym();
			do {
				vardeclaration();
				while (sym == SYM_COMMA) {
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) {
					getsym();
				}
				else {
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
			//			block = dx;
		} // if

		while (sym == SYM_PROCEDURE) { // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER) {
				enter(ID_PROCEDURE);
				getsym();
			}
			else {
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON) {
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else {
				error(5); // Missing ',' or ';'.
			}
		} // while
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

  //////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff) {
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

  //////////////////////////////////////////////////////////////////////
  // interprets and executes codes.
void interpret() {

	int pc; // program counter
	int stack[STACKSIZE];
	int top; // top of stack
	int b; // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do {
		i = code[pc++];
		switch (i.f) {
		case SCA:
			int temp;
			scanf("%d", &temp);
			stack[base(stack, b, i.l) + i.a] = temp;
			break;
		case PRT:
			printf("print:%d\n", stack[base(stack, b, i.l) + i.a]);
			break;
		case PT:
			printf("%d\n", stack[top]);
			//top--;
			break;
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0) {
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
				//newly
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_NOT:
				stack[top] = (stack[top] == 0 ? 1 : 0);
				break;


			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("stacktop: %d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;

		case JC:
			if (stack[top] != 0)
				pc = i.a;
			top--;
			break;
		case POP:
			top--;
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

void printvar() {
	int i;
	i = position(id);
	mask* mk;
	mk = (mask*)&table[i];
	gen(PRT, level - mk->level, mk->address);
	getsym();
}


void inputvar() {
	int i;
	i = position(id);
	mask* mk;
	mk = (mask*)&table[i];
	gen(SCA, level - mk->level, mk->address);
	getsym();


}
//////////////////////////////////////////////////////////////////////
void main() {
	FILE* hbin;
	char s[80] = "input.txt";
	int i;
	symset set, set1, set2;

	//	printf("Please input source file name: "); // get file name to be compiled
	//	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL) {
		printf("File %s can't be opened.\n", s);

		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_FOR, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_NOT, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);

	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0) {
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	system("pause");

} // main

  //////////////////////////////////////////////////////////////////////
  // eof pl0.c
