%option noyywrap
%option yylineno

%x MULTI_COMMENT_STATE

%{
	#include<iostream>
	#include<cstdlib>
	#include<string>
	#include<cstring>
	#include "utils/symbolTable/1805019_symbolTable.h"
	#include "y.tab.h"
	using namespace std;

	#define YYSTYPE symbolInfo*

	#define SYMBOL_TABLE_SIZE 13
	#define MAX_BUFFER_SIZE 2000

	//extern YYSTYPE yylval;

	extern symbolTable table;
	void yyerror(char *);

	extern int line_count;
	extern int error_count;
	extern FILE* errorFile;

	char buffer[MAX_BUFFER_SIZE];

	void printErrorLine() {
        fprintf(errorFile, "Error at line no %d: ", line_count);
    }

	char parseChar(char *str) {
        if(yyleng == 3) {
            return str[1];
        }
        else {
            if(str[2] == 'n') {
                return '\n';
            }
            if(str[2] == 't') {
                return '\t';
            }
            if(str[2] == 'a') {
                return '\a';
            }
            if(str[2] == 'f') {
                return '\f';
            }
            if(str[2] == 'r') {
                return '\r';
            }
            if(str[2] == 'b') {
                return '\b';
            }
            if(str[2] == 'v') {
                return '\v';
            }
            if(str[2] == '0') {
                return '\0';
            }
            if(str[2] == '\\') {
                return '\\';
            }
            if(str[2] == '\'') {
                return '\'';
            }
        }
        return '\0';
    }

    void parseString(const char* str) {
        int j = 0;
        for(int i=0;i<yyleng;i++) {
            if(str[i] == '\n') {
                // do nothing
            }
            else if(str[i] == '\\') {
                if(str[i+1] == '\n') {
                    //do nothing
                    continue;
                }
                if(str[i+1] == 'n') {
                    buffer[j++] = '\n';
                    i++;
                }
                else if(str[i+1] == '\\') {
                    buffer[j++] = '\\';
                    i++;
                }
                else if(str[i+1] == 't') {
                    buffer[j++] = '\t';
                    i++;
                }
                else if(str[i+1] == 'f') {
                    buffer[j++] = '\f';
                    i++;
                }
                else if(str[i+1] == 'a') {
                    buffer[j++] = '\a';
                    i++;
                }
                else if(str[i+1] == 'b') {
                    buffer[j++] = '\b';
                    i++;
                }
                else if(str[i+1] == 'r') {
                    buffer[j++] = '\r';
                    i++;
                }
                else if(str[i+1] == '\'') {
                    buffer[j++] = '\'';
                    i++;
                }
                else if(str[i+1] == 'v') {
                    buffer[j++] = '\v';
                    i++;
                }
                else {
                    buffer[j] = i+1 < yyleng ? str[i+1] : buffer[j];
                    i++;
                    j++;
                }
            }
            else {
                buffer[j++] = str[i];
            }
        }
        buffer[j] = '\0';
    }

    int countDot(char *str) {
        char *s = str;
        int count = 0;
        while(*s++) {
            if(*s == '.') {
                count++;
            }
        }
        return count;
    }

%}

WHITESPACE [ \t\f\r\v]+
LETTER [a-zA-Z_]
DIGIT [0-9]
NEWLINE \n
FLOAT {DIGIT}*(\.{DIGIT}+)?(E[+-]?{DIGIT}+)?
INT_EXPONEN {DIGIT}+[E]?{DIGIT}+
SPECIAL_CHAR [\n\t\a\f\r\b\v\0\\\']
SPECIAL_CHAR_ABVR [ntafrbv0\\']
IDENTIFIER ({LETTER}+)({LETTER}|{DIGIT})*
VALID_CHARACTER (.|\\n|\\t|\\a|\\f|\\r|\\b|\\v|\\0|\\|\\')
VALID_STRING ([^"\\\n]|\\.|\\\n)*

%%

{WHITESPACE}  {}

{NEWLINE}   {
                    //printf("new line\n");
                    line_count++;
            }

{DIGIT}+    {
                //printf("yessir : %s\n", yytext);
                string const_int("CONST_INT");
                string text(yytext);
                symbolInfo *info = new symbolInfo(text, const_int);
                yylval.symbol = (YYSTYPE) info;
                return CONST_INT;
            }

{FLOAT}     {
                string const_float("CONST_FLOAT");
                string text(yytext);
                symbolInfo *info = new symbolInfo(text, const_float);
                yylval.symbol = (YYSTYPE) info;
                return CONST_FLOAT;
            }

{FLOAT}+    {
                printErrorLine();
                int dotCount = countDot(yytext);
                if(dotCount > 1) {
                    fprintf(errorFile, "Too many decimal point error %s\n\n", yytext);
                }
                else {
                    fprintf(errorFile, "Ill formed number %s\n\n", yytext);
                }
                error_count++;
            }

['][']  {
            printErrorLine();
            fprintf(errorFile, "Empty character constant error %s\n\n", yytext);
            error_count++;
        }

[']{VALID_CHARACTER}?    {
                            printErrorLine();
                            fprintf(errorFile, "Unterminated character %s\n\n", yytext);
                            error_count++;
                         }

[']{VALID_CHARACTER}[']     {
                                char c = parseChar(yytext);
                                string name = "";
                                name += c;
                                string const_char("CONST_CHAR");
                                symbolInfo *info = new symbolInfo(name, const_char);
                                yylval.symbol = (YYSTYPE) info;
                                return CONST_CHAR;
                            }


[']{VALID_CHARACTER}{VALID_CHARACTER}+[']   {
                                                printErrorLine();
                                                fprintf(errorFile, "Multi character constant error %s\n\n", yytext);
                                                error_count++;
                                            }

["]{VALID_STRING}\n     {
                            printErrorLine();
                            fprintf(errorFile, "Unterminated string %s\n", yytext);
                            error_count++;
                            line_count = yylineno;
                        }

["]{VALID_STRING}["]    {
                            /*
                            parseString(yytext);
                            fprintf(logFile, "Token <STRING> Lexeme %s found --> <STRING, %s>\n\n",yytext, buffer);
                            //printf("Buff: %s\n", buffer);
                            fprintf(tokenFile, "<STRING, %s> ",buffer);
                            line_count = yylineno;
                            */
                        }

("//")(.|\\\n)*     {
                        /*
                        printLineNo();
                        fprintf(logFile, "Token <COMMENT> Lexeme %s found\n\n",yytext);
                        line_count = yylineno;
                        */
                    }

"/*"    {
            strcpy(buffer, yytext);
            BEGIN MULTI_COMMENT_STATE;
        }

<MULTI_COMMENT_STATE>.|\n   {
                                strcat(buffer, yytext);
                            }

<MULTI_COMMENT_STATE>"*/"   {
                                strcat(buffer, yytext);
                                line_count = yylineno;
                                BEGIN INITIAL;
                            }

<MULTI_COMMENT_STATE><<EOF>>  {
                                //error
                                printErrorLine();
                                fprintf(errorFile, "Unterminated comment %s\n", buffer);
                                error_count++;
                                line_count = yylineno;
                                //printSummary();
                                yyterminate();
                            }


"++"    {
            return INCOP;
        }

"--"    {
            return DECOP;
        }

[+-]    {
            string addop("ADDOP");
            string text(yytext);
            symbolInfo *info = new symbolInfo(text, addop);
            yylval.symbol = (YYSTYPE) info;
            return ADDOP;
        }

[*/%]   {
            string mulop("MULOP");
            string text(yytext);
            symbolInfo *info = new symbolInfo(text, mulop);
            yylval.symbol = (YYSTYPE) info;
            return MULOP;
        }


"<"|"<="|">"|">="|"=="|"!="     {
                                    string relop("RELOP");
                                    string text(yytext);
                                    symbolInfo *info = new symbolInfo(text, relop);
                                    yylval.symbol = (YYSTYPE) info;
                                    return RELOP;
                                }

=   {
        return ASSIGNOP;
    }

"&&"|"\|\|"     {
                    string logicop("LOGICOP");
                    string text(yytext);
                    symbolInfo *info = new symbolInfo(text, logicop);
                    yylval.symbol = (YYSTYPE) info;
                    return LOGICOP;
                }

!   {
        return NOT;
    }

\(  {
        return LPAREN;
    }

\)  {
        return RPAREN;
    }

\{  {
        return LCURL;
    }

\}  {
        return RCURL;
    }

\[  {
        return LTHIRD;
    }

\]  {
        return RTHIRD;
    }

,   {
        return COMMA;
    }

;   {
        return SEMICOLON;
    }

"printf"    {
                return PRINTLN;
            }


"if"    {
            return IF;
        }

"for"   {
            return FOR;
        }

"do"    {
            return DO;
        }

"int"   {
            return INT;
        }

"float" {
            return FLOAT;
        }

"void"  {
            return VOID;
        }

"switch"    {
                return SWITCH;
            }

"default"   {
                return DEFAULT;
            }

"else"   {
            return ELSE;
         }

"while" {
            return WHILE;
        }

"break" {
            return BREAK;
        }

"char"  {
            return CHAR;
        }

"double"    {
                return DOUBLE;
            }

"return"    {
                return RETURN;
            }

"case"  {
            return CASE;
        }

"continue"  {
                return CONTINUE;
            }

{IDENTIFIER}    {
                    string id("ID");
                    string text(yytext);
                    symbolInfo *info = new symbolInfo(text, id);
                    yylval.symbol = (YYSTYPE) info;
                    return ID;
                }

{DIGIT}+{IDENTIFIER}    {
                            printErrorLine();
                            fprintf(errorFile, "Invalid prefix on ID or invalid suffix on Number %s\n\n", yytext);
                            error_count++;
                        }

.   {
        printErrorLine();
        error_count++;
        fprintf(errorFile, "Unrecognized character %s\n\n", yytext);
    }

<<EOF>>     {

                //printSummary();
                yyterminate();

            }
%%
