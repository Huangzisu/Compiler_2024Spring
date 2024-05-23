/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Y_TAB_HPP_INCLUDED
# define YY_YY_Y_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ADD = 258,                     /* ADD  */
    SUB = 259,                     /* SUB  */
    MUL = 260,                     /* MUL  */
    DIV = 261,                     /* DIV  */
    EQ = 262,                      /* EQ  */
    NEQ = 263,                     /* NEQ  */
    LT = 264,                      /* LT  */
    GT = 265,                      /* GT  */
    LTE = 266,                     /* LTE  */
    GTE = 267,                     /* GTE  */
    AND = 268,                     /* AND  */
    OR = 269,                      /* OR  */
    NOT = 270,                     /* NOT  */
    LPARENT = 271,                 /* LPARENT  */
    RPARENT = 272,                 /* RPARENT  */
    LBRACKET = 273,                /* LBRACKET  */
    RBRACKET = 274,                /* RBRACKET  */
    LBRACE = 275,                  /* LBRACE  */
    RBRACE = 276,                  /* RBRACE  */
    DOT = 277,                     /* DOT  */
    RIGHT_ARROW = 278,             /* RIGHT_ARROW  */
    SEMICOLON = 279,               /* SEMICOLON  */
    COMMA = 280,                   /* COMMA  */
    ASSIGN = 281,                  /* ASSIGN  */
    COLON = 282,                   /* COLON  */
    STRUCT = 283,                  /* STRUCT  */
    FN = 284,                      /* FN  */
    RETURN = 285,                  /* RETURN  */
    IF = 286,                      /* IF  */
    ELSE = 287,                    /* ELSE  */
    BREAK = 288,                   /* BREAK  */
    CONTINUE = 289,                /* CONTINUE  */
    WHILE = 290,                   /* WHILE  */
    INT = 291,                     /* INT  */
    LET = 292,                     /* LET  */
    ID = 293,                      /* ID  */
    Num = 294                      /* Num  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define ADD 258
#define SUB 259
#define MUL 260
#define DIV 261
#define EQ 262
#define NEQ 263
#define LT 264
#define GT 265
#define LTE 266
#define GTE 267
#define AND 268
#define OR 269
#define NOT 270
#define LPARENT 271
#define RPARENT 272
#define LBRACKET 273
#define RBRACKET 274
#define LBRACE 275
#define RBRACE 276
#define DOT 277
#define RIGHT_ARROW 278
#define SEMICOLON 279
#define COMMA 280
#define ASSIGN 281
#define COLON 282
#define STRUCT 283
#define FN 284
#define RETURN 285
#define IF 286
#define ELSE 287
#define BREAK 288
#define CONTINUE 289
#define WHILE 290
#define INT 291
#define LET 292
#define ID 293
#define Num 294

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 22 "parser.yacc"

  A_pos pos;
  A_type type;
  A_varDecl varDecl;
  A_varDef varDef;
  A_rightVal rightVal;
  A_arithExpr arithExpr;
  A_boolExpr boolExpr;
  A_arithBiOpExpr arithBiOpExpr;
  A_arithUExpr arithUExpr;
  A_exprUnit exprUnit;
  A_fnCall fnCall;
  A_indexExpr indexExpr;
  A_arrayExpr arrayExpr;
  A_memberExpr memberExpr;
  A_boolUnit boolUnit;
  A_boolBiOpExpr boolBiOpExpr;
  A_boolUOpExpr boolUOpExpr;
  A_comExpr comExpr;
  A_leftVal leftVal;
  A_assignStmt assignStmt;
  A_rightValList rightValList;
  A_varDefScalar varDefScalar;
  A_varDefArray varDefArray;
  A_varDeclScalar varDeclScalar;
  A_varDeclArray varDeclArray;
  A_varDeclStmt varDeclStmt;
  A_varDeclList varDeclList;
  A_structDef structDef;
  A_paramDecl paramDecl;
  A_fnDecl fnDecl;
  A_fnDef fnDef;
  A_codeBlockStmt codeBlockStmt;
  A_ifStmt ifStmt;
  A_whileStmt whileStmt;
  A_fnDeclStmt fnDeclStmt;
  A_callStmt callStmt;
  A_returnStmt returnStmt;
  A_programElement programElement;
  A_codeBlockStmtList codeBlockStmtList;
  A_programElementList programElementList;
  A_program program;
  A_tokenId tokenId;
  A_tokenNum tokenNum;

#line 191 "y.tab.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_HPP_INCLUDED  */
