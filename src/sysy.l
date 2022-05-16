%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>
#include "ast.h"

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

%}

/* 空白符和注释, 块注释自行处理 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*$
BlockComment   \/\*(.|\r\n)*\*\/

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+


%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }
{BlockComment}  { /* 忽略, 不做任何操作 */ }

"int"           { return INT; }
"return"        { return RETURN; }
"+"             { yylval.str_val = new string(yytext); return PLUS; }
"-"             { yylval.str_val = new string(yytext); return MINUS; }
"!"             { yylval.str_val = new string(yytext); return SEQZ; }
"*"             { yylval.str_val = new string(yytext); return MUL; }
"/"             { yylval.str_val = new string(yytext); return DIV; }
"%"             { yylval.str_val = new string(yytext); return MOD; }
"<"             { yylval.str_val = new string(yytext); return LE; }
">"             { yylval.str_val = new string(yytext); return GE; }
"<="            { yylval.str_val = new string(yytext); return LEQ; }
">="            { yylval.str_val = new string(yytext); return GEQ; }
"=="            { yylval.str_val = new string(yytext); return EQ; }
"!="            { yylval.str_val = new string(yytext); return NEQ; }





{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.               { return yytext[0]; }

%%
