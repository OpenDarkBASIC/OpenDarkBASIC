%{
    #include "odbc/Parser.y.h"
%}

%option nodefault
%option noyywrap
%option bison-bridge
%option reentrant
%option extra-type="odbc::Driver*"

REMARK1 (?i:remstart).*(?i:remend)\n?
REMARK2 (?i:rem).*\n
REMARK3 "/*".*"*/"\n?
REMARK4 "//".*\n
REMARK ({REMARK1}|{REMARK2}|{REMARK3}|{REMARK4})

CONSTANT #constant

BOOL_TRUE       (?i:true)
BOOL_FALSE      (?i:false)
STRING_LITERAL  \".*\"
FLOAT_EXP       [eE]-?[0-9]+
FLOAT1          -?[0-9]+\.[0-9]+?
FLOAT2          -?\.[0-9]+
FLOAT3          -?[0-9]+\.[0-9]+?{FLOAT_EXP}?
FLOAT4          -?\.[0-9]+{FLOAT_EXP}?
FLOAT5          -?[0-9]+{FLOAT_EXP}
FLOAT           {FLOAT1}f?|{FLOAT2}f?|{FLOAT3}|{FLOAT4}|{FLOAT5}
INTEGER_BASE2   0b[01]+
INTEGER_BASE16  0x[0-9a-fA-F]+
INTEGER         -?[0-9]+
SYMBOL          [a-zA-Z_][a-zA-Z0-9_]+?

%%

{REMARK}            { printf("remark: %s", yytext); }

{CONSTANT}          { return TOK_CONSTANT; }

{BOOL_TRUE}         { printf("bool: %s\n", yytext); yylval->boolean_value = true; return TOK_BOOLEAN; }
{BOOL_FALSE}        { printf("bool: %s\n", yytext); yylval->boolean_value = false; return TOK_BOOLEAN; }
{STRING_LITERAL}    {
                        printf("string_literal: %s\n", yytext);
                        int len = strlen(yytext);
                        yylval->string_literal = (char*)malloc(len - 2 + 1);
                        memcpy(yylval->string_literal, &yytext[1], len - 2);
                        yylval->string_literal[len - 2] = '\0';
                        return TOK_STRING_LITERAL;
                    }
{FLOAT}             { printf("float: %s\n", yytext); yylval->float_value = atof(yytext); return TOK_FLOAT; }
{INTEGER_BASE2}     { printf("integer: %s\n", yytext); yylval->integer_value = strtol(&yytext[2], nullptr, 2); return TOK_INTEGER; }
{INTEGER_BASE16}    { printf("integer: %s\n", yytext); yylval->integer_value = strtol(&yytext[2], nullptr, 16); return TOK_INTEGER; }
{INTEGER}           { printf("integer: %s\n", yytext); yylval->integer_value = strtol(yytext, nullptr, 10); return TOK_INTEGER; }
{SYMBOL}            { printf("symbol: %s\n", yytext); yylval->symbol = strdup(yytext); return TOK_SYMBOL; }

[\n:]               { printf("end statement\n"); return TOK_END_STATEMENT; }
.                   {}
%%
