/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Scanner.h"

#define FILE_LEN 30

extern FILE *sourceFile;                       // miniC source program
extern char fileName[FILE_LEN];

int superLetter(char ch);
int superLetterOrDigit(char ch);
void getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);
int line;
int column;
int dbflag;
int num;

char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39        40       41            */
	"while",    "{",        "||",       "}",     "char",   "double", 
	/*42          43         44        45        46       47      48    */
	"for",      "do",    "switch", "case", "break", "continue", "goto",
	"chr" ,    "str" ,   "." , ":"
};

char *keyword[NO_KEYWORD] = {
	"const", "else", "if", "int", "return", "void", "while", "char", "double", "for" , "do" , "switch", "case", "break", "continue", "goto" //char~goto 9∞≥ »Æ¿Â
};

enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid, twhile, tchar, tdouble, tfor, tdo, tswitch, tcase, tbreak, tcontinue, tgoto
};

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH], temp ,str[STR_MAX],db[DB_MAX];
	double d = 0;
	num = 0;
	dbflag = 0;
	token.number = tnull;
	
	do {
		while (isspace(ch = fgetc(sourceFile))) 
		{
			if (ch == '\n')
			{
				line++;
				column = 0;
			}
		}	// state 1: skip blanks
		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
									 // find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) 
					break;
			if (index < NO_KEYWORD)    // found, keyword exit
				token.number = tnum[index];
			else {                     // not found, identifier exit
				token.number = tident;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword
		else if (isdigit(ch)) {  // number
			getNumber(ch);
			if (dbflag) // double
			{
				token.number = treal;
				token.value.db = (double)num;
				d = 10;
				ch = fgetc(sourceFile);
				ch = fgetc(sourceFile);
				do {	

					if (isspace(ch)) //short form type 1 (xxx.)
					{
						break;
					}			
					if (ch == 'e' || ch == 'E') // double using exponential
					{
						ch = fgetc(sourceFile);
						if (ch == '-')
						{
							ch = fgetc(sourceFile);
							for (int k = 0; k < hexValue(ch); k++)
								token.value.db *= 0.1;
							ch = fgetc(sourceFile);
						}
						else if (ch == '+')
						{
							ch = fgetc(sourceFile);
							for (int k = 0; k < hexValue(ch); k++)
								token.value.db *= 10;
							ch = fgetc(sourceFile);
						}
						else if (isdigit(ch))
						{
							for (int k = 0; k < hexValue(ch); k++)
								token.value.db *= 10;
							ch = fgetc(sourceFile);
						}
						else 
						{
							lexicalError(6);
						}
					}
					else // not using exponential
					{
						token.value.db += ((1 / d) * hexValue(ch));
						d *= 10;
						ch = fgetc(sourceFile);
					}

				} while (isdigit(ch) || ch=='e' || ch=='E');
				ungetc(ch, sourceFile);
			}
			else 
			{
				token.number = tnumber;
				token.value.num = num;
			}
		}
		else switch (ch) {  // special character
		case '/':
			ch = fgetc(sourceFile);
			if (ch == '*')			// text comment
			{
				ch = fgetc(sourceFile);
				if (ch == '*') // documented comment
				{	
					printf("Documented Comments ------>");
					do {
						while (ch != '*')
						{
							if (ch == '\n')
							{
								line++;
								column = 0;
							}
							putchar(ch);
							ch = fgetc(sourceFile);						
						}
						ch = fgetc(sourceFile);
					} while (ch != '/');
				}
				printf("\n");
				line++;
				column = 0;
			}
			else if (ch == '/')		// single line comment
			{
				ch = fgetc(sourceFile);
				if (ch == '/')
				{
					ch = fgetc(sourceFile);
					printf("Documented Comments ------>");
					do 
					{
						putchar(ch);
						ch = fgetc(sourceFile);
					} while (ch != '\n');
				}
				printf("\n");
				line++;
				column = 0;
			}
			else if (ch == '=')  token.number = tdivAssign;
			else {
				token.number = tdiv;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '!':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tnotequ;
			else {
				token.number = tnot;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '%':
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				ungetc(ch, sourceFile);
			}
			break;
		case '&':
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '*':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '+':
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '-':
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '<':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '=':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '>':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '|':
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '\'': // character literal
			ch = fgetc(sourceFile);
			temp = ch; // save next of '
			if (ch == '\\')  // case of escape sequence
			{
				ch = fgetc(sourceFile);
				if (ch == '0' || ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't' || ch == 'v' || ch == '\\' || ch == '\'' || ch == '\"')
				{
					token.number = tchr;
					token.value.chara[0] = temp;
					token.value.chara[1] = ch;
					ch = fgetc(sourceFile);
				}
				else
				{
					lexicalError(5);
				}		
				
			}
			else if (isalnum(ch)) // not a escape sequence
			{
				ch = fgetc(sourceFile);
				token.number = tchr;
				token.value.chara[0] = temp;
				token.value.chara[1] = ' ';
				ch = fgetc(sourceFile);
				
			}
			else {  //default
				lexicalError(4);
			}
		break;
		case '\"': // string literal
		{
			i = 0;
			do {
				ch = fgetc(sourceFile);
				str[i++] = ch;
			} while (ch != '\"');
			str[i - 1] = '\0';

			token.number = tstr;
			strcpy_s(token.value.str,str);
		}
		case '.': // short form type 2 ( .xxx)
		{
			ch = fgetc(sourceFile);
			token.number = treal;
			token.value.db = 0.0;
			d = 10;
			do {
				if (ch == 'e' || ch == 'E') // double using exponential
				{
					ch = fgetc(sourceFile);
					if (ch == '-')
					{
						ch = fgetc(sourceFile);
						for (int k = 0; k < hexValue(ch); k++)
							token.value.db *= 0.1;
						ch = fgetc(sourceFile);
					}
					else if (ch == '+')
					{
						ch = fgetc(sourceFile);
						for (int k = 0; k < hexValue(ch); k++)
							token.value.db *= 10;
						ch = fgetc(sourceFile);
					}
					else if (isdigit(ch))
					{
						for (int k = 0; k < hexValue(ch); k++)
							token.value.db *= 10;
						ch = fgetc(sourceFile);
					}
					else
					{
						lexicalError(6);
					}
				}
				else // not using exponential
				{
					token.value.db += ((1 / d) * hexValue(ch));
					d *= 10;
					ch = fgetc(sourceFile);
				}

			} while (isdigit(ch) || ch == 'e' || ch == 'E');
			ungetc(ch, sourceFile);
		}
		break;
		case '(': token.number = tlparen;         break;
		case ')': token.number = trparen;         break;
		case ',': token.number = tcomma;          break;
		case ';': token.number = tsemicolon;      break;
		case '[': token.number = tlbracket;       break;
		case ']': token.number = trbracket;       break;
		case '{': token.number = tlbrace;         break;
		case '}': token.number = trbrace;         break;
		case ':': token.number = tcolon;         break;
		case EOF: token.number = teof;            break;
		default: {
			printf("Current character : %c", ch);
			lexicalError(4);
			break;
		}

		} // switch end
		column++;
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("not a escape sequence form \n");
		break;
	case 6: printf("not a correct real form \n");
		break;
	}
}


int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

void getNumber(char firstCharacter)
{
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
			if (ch == '.')
			{
				dbflag = 1;
			}
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}


void printToken(struct tokenType token)
{
	if (token.number == tident)
		printf("%s (number: %d, value: %s, fileName: %s, lineNumber: %d, columnNumber: %d)\n", token.value.id, token.number, token.value.id, fileName,line,column);
	else if (token.number == tnumber)
		printf("%d (number: %d, value: %d, fileName: %s, lineNumber: %d, columnNumber: %d)\n", token.value.num, token.number, token.value.num, fileName, line, column);
	else if (token.number == tchr)
	{
		
		for (int i = 0; i < sizeof(token.value.chara); i++)
		{
			printf("%c", token.value.chara[i]);
		}
		printf("(number: %d, value: ", token.number);
		for (int i = 0; i < sizeof(token.value.chara); i++)
		{
			printf("%c", token.value.chara[i]);
		}
		printf(", fileName: %s, lineNumber: %d, columnNumber: %d)", fileName, line, column);
		printf("\n");
	}
	else if (token.number == tstr)
		printf("%s (number: %d, value: %s, fileName: %s, lineNumber: %d, columnNumber: %d)\n", token.value.str, token.number, token.value.str, fileName, line, column);
	else if (token.number == treal)
		printf("%lf (number: %d, value: %lf, fileName: %s, lineNumber: %d, columnNumber: %d)\n", token.value.db, token.number, token.value.db, fileName, line, column);
	else
		printf("%s (number: %d, value : %s, fileName: %s, lineNumber: %d, columnNumber: %d)\n", tokenName[token.number], token.number, tokenName[token.number], fileName, line, column);

}