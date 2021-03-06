#ifndef PL_H
#define PL_H
#include <stdio.h>

#define NRW        19    // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       10     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

enum symtype {
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_COMMENT,
	SYM_ELSE,
	SYM_LSQUARE,
	SYM_RSQUARE,
	SYM_AND,
	SYM_OR,
	SYM_NOT,
	SYM_FOR,
	SYM_BREAK,
	SYM_EXIT,
	SYM_PRINTF,
	SYM_INPUT
};

enum idtype {
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE
};

enum opcode {
	//将常数置于栈顶
	LIT,
	//算术或逻辑运算指令
	OPR,
	//将变量值置于栈顶
	LOD,
	//将栈顶的值赋予变量
	STO,
	//过程调用
	CAL,
	//在数据栈中分配存储空间
	INT,
	JMP,
	JPC,
	JC,
	POP,

	PRT,
	SCA,
	PT

};

enum oprcode {
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_AND, OPR_OR, OPR_NOT
};


typedef struct {
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
	/*  0 */    "",
	/*  1 */    "Found ':=' when expecting '='.",
	/*  2 */    "There must be a number to follow '='.",
	/*  3 */    "There must be an '=' to follow the identifier.",
	/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */    "Missing ',' or ';'.",
	/*  6 */    "Incorrect procedure name.",
	/*  7 */    "Statement expected.",
	/*  8 */    "Follow the statement is an incorrect symbol.",
	/*  9 */    "'.' expected.",
	/* 10 */    "';' expected.",
	/* 11 */    "Undeclared identifier.",
	/* 12 */    "Illegal assignment.",
	/* 13 */    "':=' expected.",
	/* 14 */    "There must be an identifier to follow the 'call'.",
	/* 15 */    "A constant or variable can not be called.",
	/* 16 */    "'then' expected.",
	/* 17 */    "';' or 'end' expected.",
	/* 18 */    "'do' expected.",
	/* 19 */    "Incorrect symbol.",
	/* 20 */    "Relative operators expected.",
	/* 21 */    "Procedure identifier can not be in an expression.",
	/* 22 */    "Missing ')'.",
	/* 23 */    "The symbol can not be followed by a factor.",
	/* 24 */    "The symbol can not be as the beginning of an expression.",
	/* 25 */    "The number is too great.",
	/* 26 */    "",
	/* 27 */    "",
	/* 28 */    "",
	/* 29 */    "",
	/* 30 */    "",
	/* 31 */    "",
	/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;	// 指示现在变量表中的下一个元素
int base_dim = 1;//指示数组扩展时当前单位量

char line[80];

instruction code[CXMAX];
//预留字，0为哨兵
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while","else","[","]","for","break","exit","print","input"
};
//与预留字对应的宏变量
int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,SYM_ELSE,SYM_LSQUARE,SYM_RSQUARE,SYM_FOR,SYM_BREAK,SYM_EXIT,SYM_PRINTF,SYM_INPUT
};
//与运算符对应的宏
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON//SYM_AND,SYM_OR,SYM_NOT
};
//运算符
char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';'//'&','|','!'
};

#define MAXINS   13
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC","JC","POP","PRT","SCA","PT"
};

//变量表,包含名字,类型和值
typedef struct {
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

comtab table[TXMAX];


//定义了一个实体的信息,包括名字,类型,层次和地址
typedef struct {
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
} mask;

FILE* infile;
#endif

// EOF PL0.h
