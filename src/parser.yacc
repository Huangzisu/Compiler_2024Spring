%{
#include <stdio.h>
#include <string.h>
#include "TeaplAst.h"

extern A_pos pos;
extern A_program root;

extern int yylex(void);
extern "C"{
extern void yyerror(char *s); 
extern int  yywrap();
}

%}


// TODO:
// your parser

// referring to definitions in TeaplAst.h
%union {
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
}

// tokens
%token <pos> ADD  // +
%token <pos> SUB  // -
%token <pos> MUL  // x
%token <pos> DIV  // /
%token <pos> EQ   // ==
%token <pos> NEQ  // !=
%token <pos> LT   // <
%token <pos> GT   // >
%token <pos> LTE  // <=
%token <pos> GTE  // >=
%token <pos> AND  // &&
%token <pos> OR   // ||
%token <pos> NOT  // !
%token <pos> LPARENT  // (
%token <pos> RPARENT  // )
%token <pos> LBRACKET  // [
%token <pos> RBRACKET  // ]
%token <pos> LBRACE  // {
%token <pos> RBRACE  // }
%token <pos> DOT  // .
%token <pos> RIGHT_ARROW  // ->
%token <pos> SEMICOLON  // ;
%token <pos> COMMA  // ,
%token <pos> ASSIGN  // =
%token <pos> COLON  // :
%token <pos> STRUCT  // struct
%token <pos> FN  // fn
%token <pos> RETURN  // return
%token <pos> IF  // if
%token <pos> ELSE  // else
%token <pos> BREAK // break
%token <pos> CONTINUE // continue
%token <pos> WHILE  // while
%token <pos> INT  // int
%token <pos> LET // let
%token <tokenId> ID
%token <tokenNum> Num

// non-terminals
%type <type> Type
%type <varDecl> VarDecl
%type <varDef> VarDef
%type <rightVal> RightVal
%type <arithExpr> ArithExpr
%type <boolExpr> BoolExpr
%type <arithBiOpExpr> ArithBiOpExpr
%type <arithUExpr> ArithUExpr
%type <exprUnit> ExprUnit
%type <fnCall> FnCall
%type <indexExpr> IndexExpr
%type <arrayExpr> ArrayExpr
%type <memberExpr> MemberExpr
%type <boolUnit> BoolUnit
%type <boolBiOpExpr> BoolBiOpExpr
%type <boolUOpExpr> BoolUOpExpr
%type <comExpr> ComExpr
%type <leftVal> LeftVal
%type <assignStmt> AssignStmt
%type <rightValList> RightValList
%type <varDefScalar> VarDefScalar
%type <varDefArray> VarDefArray
%type <varDeclScalar> VarDeclScalar
%type <varDeclArray> VarDeclArray
%type <varDeclStmt> VarDeclStmt
%type <varDeclList> VarDeclList
%type <structDef> StructDef
%type <paramDecl> ParamDecl
%type <fnDecl> FnDecl
%type <fnDef> FnDef
%type <codeBlockStmt> CodeBlockStmt
%type <ifStmt> IfStmt
%type <whileStmt> WhileStmt
%type <fnDeclStmt> FnDeclStmt
%type <callStmt> CallStmt
%type <returnStmt> ReturnStmt
%type <programElement> ProgramElement
%type <codeBlockStmtList> CodeBlockStmtList
%type <programElementList> ProgramElementList
%type <program> Program

%start Program

%left ADD SUB AND OR
%left MUL DIV NOT

// rules, referring to definitions in /Spec/teapl.md
%%

// nativeType | structType 
Type: INT
      {
        $$ = A_NativeType($1, A_intTypeKind);
      }
      |
      ID
      {
        $$ = A_StructType($1->pos, $1->id);
      };

// id < : > type | id < [ > num < ] >< : > type
VarDecl: VarDeclScalar
        {
          $$ = A_VarDecl_Scalar($1->pos, $1);
        }
        | 
        VarDeclArray
        {
          $$ = A_VarDecl_Array($1->pos, $1);
        };

// id < : > type < = > rightVal | id < [ > num < ] >< : > type < = > < { > rightVal (< , > rightVal)* < } >
VarDef: VarDefScalar
        {
          $$ = A_VarDef_Scalar($1->pos, $1);
        }
        |
        VarDefArray
        {
          $$ = A_VarDef_Array($1->pos, $1);
        };

// arithExpr | boolExpr
RightVal: ArithExpr
          {
            $$ = A_ArithExprRVal($1->pos, $1);
          }
          |
          BoolExpr
          {
            $$ = A_BoolExprRVal($1->pos, $1);
          };

// arithBiOpExpr | exprUnit
ArithExpr: ArithBiOpExpr
          {
            $$ = A_ArithBiOp_Expr($1->pos, $1);
          }
          |
          ExprUnit
          {
            $$ = A_ExprUnit($1->pos, $1);
          };

// boolBiOpExpr | boolUnit
BoolExpr: BoolBiOpExpr
          {
            $$ = A_BoolBiOp_Expr($1->pos, $1);
          }
          |
          BoolUnit
          {
            $$ = A_BoolExpr($1->pos, $1);
          };

// arithExpr arithBiOp arithExpr
ArithBiOpExpr: ArithExpr ADD ArithExpr
              {
                $$ = A_ArithBiOpExpr($1->pos, A_add, $1, $3);
              }
              |
              ArithExpr SUB ArithExpr
              {
                $$ = A_ArithBiOpExpr($1->pos, A_sub, $1, $3);
              }
              |
              ArithExpr MUL ArithExpr
              {
                $$ = A_ArithBiOpExpr($1->pos, A_mul, $1, $3);
              }
              |
              ArithExpr DIV ArithExpr
              {
                $$ = A_ArithBiOpExpr($1->pos, A_div, $1, $3);
              };

// arithUOp exprUnit
ArithUExpr: SUB ExprUnit 
            {
              $$ = A_ArithUExpr($1, A_neg, $2);
            };

// num | id | < ( > arithExpr < ) > | fnCall | id < [ > id | num < ] > | id < . > id | arithUExpr
ExprUnit: Num
          {
            $$ = A_NumExprUnit($1->pos, $1->num);
          }
          |
          ID
          {
            $$ = A_IdExprUnit($1->pos, $1->id);
          }
          |
          LPARENT ArithExpr RPARENT
          {
            $$ = A_ArithExprUnit($1, $2);
          }
          |
          FnCall
          {
            $$ = A_CallExprUnit($1->pos, $1);
          }
          |
          ArrayExpr
          {
            $$ = A_ArrayExprUnit($1->pos, $1);
          }
          |
          MemberExpr
          {
            $$ = A_MemberExprUnit($1->pos, $1);
          }
          |
          ArithUExpr
          {
            $$ = A_ArithUExprUnit($1->pos, $1);
          };

// id < ( > rightValList < ) >
FnCall: ID LPARENT RightValList RPARENT
        {
          $$ = A_FnCall($1->pos, $1->id, $3);
        };

// id | num
IndexExpr: Num
          {
            $$ = A_NumIndexExpr($1->pos, $1->num);
          }
          |
          ID
          {
            $$ = A_IdIndexExpr($1->pos, $1->id);
          };

// leftVal < [ > indexExpr < ] >
ArrayExpr: LeftVal LBRACKET IndexExpr RBRACKET
          {
            $$ = A_ArrayExpr($1->pos, $1, $3);
          };

// leftVal < . > id
MemberExpr: LeftVal DOT ID
          {
            $$ = A_MemberExpr($1->pos, $1, $3->id);
          };

//  comExpr | < ( > boolExpr < ) > | boolUOp
BoolUnit: ComExpr
          {
            $$ = A_ComExprUnit($1->pos,$1);
          }
          |
          LPARENT BoolExpr RPARENT
          {
            $$ = A_BoolExprUnit($1, $2);
          }
          |
          BoolUOpExpr
          {
            $$ = A_BoolUOpExprUnit($1->pos, $1);
          };

// boolExpr boolBiOp boolUnit
BoolBiOpExpr: BoolExpr AND BoolExpr
              {
                $$ = A_BoolBiOpExpr($1->pos, A_and, $1, $3);
              }
              |
              BoolExpr OR BoolExpr
              {
                $$ = A_BoolBiOpExpr($1->pos, A_or, $1, $3);
              };

// < ! > boolUnit
BoolUOpExpr: NOT BoolUnit
            {
              $$ = A_BoolUOpExpr($1, A_not, $2);
            };

// comOp := < > > | < < > | < >= > | < <= > | < == > | < != >
ComExpr: ExprUnit GT ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_gt, $1, $3);
        }
        |
        ExprUnit LT ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_lt, $1, $3);
        }
        |
        ExprUnit GTE ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_ge, $1, $3);
        }
        |
        ExprUnit LTE ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_le, $1, $3);
        }
        |
        ExprUnit EQ ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_eq, $1, $3);
        }
        |
        ExprUnit NEQ ExprUnit
        {
          $$ = A_ComExpr($1->pos, A_ne, $1, $3);
        };

LeftVal: ID
        {
          $$ = A_IdExprLVal($1->pos, $1->id);
        }
        |
        ArrayExpr
        {
          $$ = A_ArrExprLVal($1->pos, $1);
        }
        |
        MemberExpr
        {
          $$ = A_MemberExprLVal($1->pos, $1);
        };

AssignStmt: LeftVal ASSIGN RightVal SEMICOLON
            {
              $$ = A_AssignStmt($1->pos, $1, $3);
            };

//  rightVal (< , > rightVal)* | ϵ
RightValList: {
                $$ = NULL;
              }
              |
              RightVal
              {
                $$ = A_RightValList($1, NULL);
              }
              |
              RightVal COMMA RightValList
              {
                $$ = A_RightValList($1, $3);
              };

// id < : > type < = > rightVal
VarDefScalar: ID COLON Type ASSIGN RightVal
              {
                $$ = A_VarDefScalar($1->pos, $1->id, $3, $5);
              };

//  id < [ > num < ] >< : > type < = > < { > rightVal (< , > rightVal)* | ϵ < } > | id < [ > num < ] > < = > < { > rightVal (< , > rightVal)* | ϵ < } >
VarDefArray: ID LBRACKET Num RBRACKET COLON Type ASSIGN LBRACE RightValList RBRACE
            {
              $$ = A_VarDefArray($1->pos, $1->id, $3->num, $6, $9);
            }
            |
            ID LBRACKET Num RBRACKET ASSIGN LBRACE RightValList RBRACE
            {
              $$ = A_VarDefArray($1->pos, $1->id, $3->num, NULL, $7);
            };

// id < : > type
VarDeclScalar: ID COLON Type
              {
                $$ = A_VarDeclScalar($1->pos, $1->id, $3);
              };

// id < [ > num < ] >< : > type
VarDeclArray: ID LBRACKET Num RBRACKET COLON Type
              {
                $$ = A_VarDeclArray($1->pos, $1->id, $3->num, $6);
              };

// < let > (varDecl | varDef) < ; >
VarDeclStmt: LET VarDecl SEMICOLON
            {
              $$ = A_VarDeclStmt($1, $2);
            }
            |
            LET VarDef SEMICOLON
            {
              $$ = A_VarDefStmt($1, $2);
            };

// varDecl (< , > varDecl)* | ϵ
VarDeclList:{
              $$ = NULL;
            }
            |
            VarDecl
            {
              $$ = A_VarDeclList($1, NULL);
            }
            |
            VarDecl COMMA VarDeclList
            {
              $$ = A_VarDeclList($1, $3);
            };

// < struct > id < { > (varDecl) (< , > varDecl)* < } >
StructDef: STRUCT ID LBRACE VarDeclList RBRACE
          {
            $$ = A_StructDef($1, $2->id, $4);
          };

// varDecl (< , > varDecl)* | ϵ
ParamDecl: VarDeclList
          {
            $$ = A_ParamDecl($1);
          };

//  < fn > id < ( > paramDecl < ) > | < fn > id < ( > paramDecl < ) > < -> > type
FnDecl: FN ID LPARENT ParamDecl RPARENT
        {
          $$ = A_FnDecl($1, $2->id, $4, NULL);
        }
        |
        FN ID LPARENT ParamDecl RPARENT RIGHT_ARROW Type
        {
          $$ = A_FnDecl($1, $2->id, $4, $7);
        };

// In /Spec/teapl.md , fnDef := fnDecl codeBlock
// codeBlock here equals to < { > codeBlockList < } >
FnDef: FnDecl LBRACE CodeBlockStmtList RBRACE
      {
        $$ = A_FnDef($1->pos, $1, $3);
      };

// varDeclStmt | assignStmt | callStmt | ifStmt | whileStmt | returnStmt | continueStmt | breakStmt | < ; > 
CodeBlockStmt: VarDeclStmt
              {
                $$ = A_BlockVarDeclStmt($1->pos, $1);
              }
              |
              AssignStmt
              {
                $$ = A_BlockAssignStmt($1->pos, $1);
              }
              |
              CallStmt
              {
                $$ = A_BlockCallStmt($1->pos, $1);
              }
              |
              IfStmt
              {
                $$ = A_BlockIfStmt($1->pos, $1);
              }
              |
              WhileStmt
              {
                $$ = A_BlockWhileStmt($1->pos, $1);
              }
              |
              ReturnStmt
              {
                $$ = A_BlockReturnStmt($1->pos, $1);
              }
              |
              CONTINUE SEMICOLON
              {
                $$ = A_BlockContinueStmt($1);
              }
              |
              BREAK SEMICOLON
              {
                $$ = A_BlockBreakStmt($1);
              }
              |
              SEMICOLON
              {
                $$ = A_BlockNullStmt($1);
              };
            
//  < if > < ( > boolExpr < ) > codeBlock ( < else > codeBlock | ϵ )
IfStmt: IF LPARENT BoolExpr RPARENT LBRACE CodeBlockStmtList RBRACE
        {
          $$ = A_IfStmt($1, $3, $6, NULL);
        }
        |
        IF LPARENT BoolExpr RPARENT LBRACE CodeBlockStmtList RBRACE ELSE LBRACE CodeBlockStmtList RBRACE
        {
          $$ = A_IfStmt($1, $3, $6, $10);
        };

// < while > < ( > boolExpr < ) > codeBlock
WhileStmt: WHILE LPARENT BoolExpr RPARENT LBRACE CodeBlockStmtList RBRACE
          {
            $$ = A_WhileStmt($1, $3, $6);
          };

// fnDecl < ; >
FnDeclStmt: FnDecl SEMICOLON
            {
              $$ = A_FnDeclStmt($1->pos, $1);
            };

// fnCall < ; >
CallStmt: FnCall SEMICOLON
          {
            $$ = A_CallStmt($1->pos, $1);
          };

// < ret > rightVal < ; >
ReturnStmt: RETURN RightVal SEMICOLON
            {
              $$ = A_ReturnStmt($1, $2);
            }
            |
            RETURN SEMICOLON
            {
              $$ = A_ReturnStmt($1, NULL);
            }

// varDeclStmt | structDef | fnDeclStmt | fnDef | comment | < ; >
// comments have already been handled in lexer
ProgramElement: VarDeclStmt
                {
                  $$ = A_ProgramVarDeclStmt($1->pos, $1);
                }
                |
                StructDef
                {
                  $$ = A_ProgramStructDef($1->pos, $1);
                }
                |
                FnDeclStmt
                {
                  $$ = A_ProgramFnDeclStmt($1->pos, $1);
                }
                |
                FnDef
                {
                  $$ = A_ProgramFnDef($1->pos, $1);
                }
                |
                SEMICOLON
                {
                  $$ = A_ProgramNullStmt($1);
                };

CodeBlockStmtList:{
                    $$ = NULL;
                  }
                  |
                  CodeBlockStmt CodeBlockStmtList
                  {
                    $$ = A_CodeBlockStmtList($1, $2);
                  };

ProgramElementList: {
                      $$ = NULL;
                    }
                    |
                    ProgramElement ProgramElementList
                    {
                      $$ = A_ProgramElementList($1, $2);
                    };

Program: ProgramElementList
        {
          root = A_Program($1);
          $$ = A_Program($1);
        };



%%



extern "C"{
void yyerror(char * s)
{
  fprintf(stderr, "%s\n",s);
  extern int yylineno;	// defined and maintained in lex
	extern char *yytext;	// defined and maintained in lex
	int len=strlen(yytext);
	int i;
	char buf[512]={0};
	for (i=0;i<len;++i)
	{
		sprintf(buf,"%s%d ",buf,yytext[i]);
	}
	fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
	yyparse();
}
int yywrap()
{
  return(1);
}
}


