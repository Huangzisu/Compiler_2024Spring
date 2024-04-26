%{
#include <stdio.h>
#include <string.h>
#include "TeaplAst.h"
#include "y.tab.hpp"
extern int line, col;
%}



%start COMMENT1 COMMENT2
%%

<INITIAL>{
"+" { yylval.pos = A_Pos(line, col); col += yyleng; return ADD; }
"-" { yylval.pos = A_Pos(line, col); col += yyleng; return SUB; }
"*" { yylval.pos = A_Pos(line, col); col += yyleng; return MUL; }
"/" { yylval.pos = A_Pos(line, col); col += yyleng; return DIV; }
">=" { yylval.pos = A_Pos(line, col); col += yyleng; return GTE; }
">" { yylval.pos = A_Pos(line, col); col += yyleng; return GT; }
"<=" { yylval.pos = A_Pos(line, col); col += yyleng; return LTE; }
"<" { yylval.pos = A_Pos(line, col); col += yyleng; return LT; } 
"!=" { yylval.pos = A_Pos(line, col); col += yyleng; return NEQ; }
"==" { yylval.pos = A_Pos(line, col); col += yyleng; return EQ; }
"(" { yylval.pos = A_Pos(line, col); col += yyleng; return LPARENT; }
")" { yylval.pos = A_Pos(line, col); col += yyleng; return RPARENT; } 
"[" { yylval.pos = A_Pos(line, col); col += yyleng; return LBRACKET; }
"]" { yylval.pos = A_Pos(line, col); col += yyleng; return RBRACKET; }
"{" { yylval.pos = A_Pos(line, col); col += yyleng; return LBRACE; }
"}" { yylval.pos = A_Pos(line, col); col += yyleng; return RBRACE; }
"=" { yylval.pos = A_Pos(line, col); col += yyleng; return ASSIGN; }
"," { yylval.pos = A_Pos(line, col); col += yyleng; return COMMA; }
";" { yylval.pos = A_Pos(line, col); col += yyleng; return SEMICOLON; }
":" { yylval.pos = A_Pos(line, col); col += yyleng; return COLON; }
"let" { yylval.pos = A_Pos(line, col); col += yyleng; return LET; }
"." { yylval.pos = A_Pos(line, col); col += yyleng; return DOT; }
"ret" { yylval.pos = A_Pos(line, col); col += yyleng; return RETURN; }
"&&" { yylval.pos = A_Pos(line, col); col += yyleng; return AND; }
"||" { yylval.pos = A_Pos(line, col); col += yyleng; return OR; }
"!" { yylval.pos = A_Pos(line, col); col += yyleng; return NOT; }
"while" { yylval.pos = A_Pos(line, col); col += yyleng; return WHILE; }
"break" { yylval.pos = A_Pos(line, col); col += yyleng; return BREAK; }
"continue" { yylval.pos = A_Pos(line, col); col += yyleng; return CONTINUE; }
"->" { yylval.pos = A_Pos(line, col); col += yyleng; return RIGHT_ARROW; }
"int" { yylval.pos = A_Pos(line, col); col += yyleng; return INT; }
"struct" { yylval.pos = A_Pos(line, col); col += yyleng; return STRUCT; }
"fn" { yylval.pos = A_Pos(line, col); col += yyleng; return FN; }
"if" { yylval.pos = A_Pos(line, col); col += yyleng; return IF; }
"else" { yylval.pos = A_Pos(line, col); col += yyleng; return ELSE; }
" " {col += 1;}
"\t" {col += 4;}
"\n" {line += 1; col = 1;}
[1-9][0-9]*|0 {
    yylval.tokenNum = A_TokenNum(A_Pos(line, col), atoi(yytext));
    col += yyleng;
    return Num;
}
[a-z_A-Z][a-z_A-Z0-9]* {
    char * text = (char *)malloc((yyleng + 1)*sizeof(char));
    strcpy(text, yytext);
    text[yyleng] = '\0';
    yylval.tokenId = A_TokenId(A_Pos(line, col), text);
    col+=yyleng;
    return ID;
}
"//" {BEGIN COMMENT1;}
"/*" {BEGIN COMMENT2;}
}
<COMMENT1>{
"\n" { line += 1; col = 1; BEGIN INITIAL;}
[^\n\t]   {col += 1;}
"\t"   {col += 4;}
}
<COMMENT2>{
"*/" {BEGIN INITIAL;}
"\n" {line += 1; col =1;}
[^\n\t]   {col += 1;}
"\t"   {col += 4;}
}
.	{ printf("Illegal Character:%s\n",yytext); }
%%

void update_pos(){
    yylval.pos = A_Pos(line, col);col += yyleng;
}

void update_line(int num){
    line += num;
    col = 1;
}

void update_col(int num){
    col += num;
}