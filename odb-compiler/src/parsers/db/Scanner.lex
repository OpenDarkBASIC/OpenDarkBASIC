%{
#define YYSTYPE DBSTYPE
#define YYLTYPE DBLTYPE

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

#include "odb-compiler/parsers/db/Parser.y.h"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-sdk/Str.hpp"

#define driver (static_cast<odb::db::Driver*>(dbget_extra(yyg)))
#if defined(ODBCOMPILER_VERBOSE_FLEX)
#   define dbg(text) fprintf(stderr, text ": \"%s\"\n", yytext)
#else
#   define dbg(text)
#endif

#define RETURN_TOKEN(token) do {                                              \
        dbg(#token);                                                          \
        return token;                                                         \
    } while(0)
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="odb::db::Driver*"
%option prefix="db"

REMARK          ((?i:rem)|"`"|"//")
REMARK_START    ((?i:remstart)|"/*")
REMARK_END      ((?i:remend)|"*/")

CONSTANT        #constant

BOOL_TRUE       (?i:true)
BOOL_FALSE      (?i:false)
STRING_LITERAL  \"[^"]*\"
FLOAT_EXP       [eE]-?[0-9]+
FLOAT1          [0-9]+\.[0-9]+?
FLOAT2          \.[0-9]+
FLOAT3          [0-9]+\.[0-9]+?{FLOAT_EXP}?
FLOAT4          \.[0-9]+{FLOAT_EXP}?
FLOAT5          [0-9]+{FLOAT_EXP}
FLOAT           {FLOAT1}f?|{FLOAT2}f?|{FLOAT3}|{FLOAT4}|{FLOAT5}
INTEGER_BASE2   %[01]+
INTEGER_BASE16  0x[0-9a-fA-F]+
INTEGER         [0-9]+
SYMBOL          [a-zA-Z_][a-zA-Z0-9_]+?

%x MULTI_COMMENT
%x MULTI_COMMENT_C
%x SINGLE_COMMENT

%%
<INITIAL>{
    (?i:remstart)       { BEGIN(MULTI_COMMENT); dbg("multiline remark"); }
    "/*"                { BEGIN(MULTI_COMMENT_C); dbg("multiline remark"); }
    {REMARK}            { BEGIN(SINGLE_COMMENT); dbg("single line remark"); }
}
<SINGLE_COMMENT>{
    .
    \n                  { BEGIN(INITIAL); }
}
<MULTI_COMMENT>{
    (?i:remend)         { BEGIN(INITIAL); dbg("multiline remark end"); }
    .
    \n
}
<MULTI_COMMENT_C>{
    "*/"                { BEGIN(INITIAL); dbg("multiline remark end"); }
    .
    \n
}

<INITIAL>{
    {CONSTANT}          { RETURN_TOKEN(TOK_CONSTANT); }

    {BOOL_TRUE}         { yylval->boolean_value = true; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {BOOL_FALSE}        { yylval->boolean_value = false; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {STRING_LITERAL}    { yylval->string = odb::str::newCStrRange(yytext, 1, strlen(yytext) - 1); RETURN_TOKEN(TOK_STRING_LITERAL); }
    {FLOAT}             { yylval->float_value = atof(yytext); RETURN_TOKEN(TOK_FLOAT_LITERAL); }
    {INTEGER_BASE2}     { yylval->integer_value = strtol(&yytext[2], nullptr, 2); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER_BASE16}    { yylval->integer_value = strtol(&yytext[2], nullptr, 16); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER}           { yylval->integer_value = strtol(yytext, nullptr, 10); RETURN_TOKEN(TOK_INTEGER_LITERAL); }

    "+"                 { RETURN_TOKEN('+'); }
    "-"                 { RETURN_TOKEN('-'); }
    "*"                 { RETURN_TOKEN('*'); }
    "/"                 { RETURN_TOKEN('/'); }
    (?i:mod)            { RETURN_TOKEN(TOK_MOD); }
    "^"                 { RETURN_TOKEN('^'); }
    "("                 { RETURN_TOKEN('('); }
    ")"                 { RETURN_TOKEN(')'); }
    ","                 { RETURN_TOKEN(','); }
    (?i:inc)            { RETURN_TOKEN(TOK_INC); }
    (?i:dec)            { RETURN_TOKEN(TOK_DEC); }

    "<<"                { RETURN_TOKEN(TOK_BSHL); }
    ">>"                { RETURN_TOKEN(TOK_BSHR); }
    "||"                { RETURN_TOKEN(TOK_BOR); }
    "&&"                { RETURN_TOKEN(TOK_BAND); }
    "~~"                { RETURN_TOKEN(TOK_BXOR); }
    ".."                { RETURN_TOKEN(TOK_BNOT); }

    "<>"                { RETURN_TOKEN(TOK_NE); }
    "<="                { RETURN_TOKEN(TOK_LE); }
    ">="                { RETURN_TOKEN(TOK_GE); }
    "="                 { RETURN_TOKEN('='); }
    "<"                 { RETURN_TOKEN('<'); }
    ">"                 { RETURN_TOKEN('>'); }

    (?i:xor)            { RETURN_TOKEN(TOK_LXOR); }
    (?i:or)             { RETURN_TOKEN(TOK_LOR); }
    (?i:and)            { RETURN_TOKEN(TOK_LAND); }
    (?i:not)            { RETURN_TOKEN(TOK_LNOT); }

    (?i:then)           { RETURN_TOKEN(TOK_THEN); }
    (?i:endif)          { RETURN_TOKEN(TOK_ENDIF); }
    (?i:elseif)         { RETURN_TOKEN(TOK_ELSEIF); }
    (?i:if)             { RETURN_TOKEN(TOK_IF); }
    (?i:else)           { RETURN_TOKEN(TOK_ELSE); }
    (?i:endwhile)       { RETURN_TOKEN(TOK_ENDWHILE); }
    (?i:while)          { RETURN_TOKEN(TOK_WHILE); }
    (?i:repeat)         { RETURN_TOKEN(TOK_REPEAT); }
    (?i:until)          { RETURN_TOKEN(TOK_UNTIL); }
    (?i:do)             { RETURN_TOKEN(TOK_DO); }
    (?i:loop)           { RETURN_TOKEN(TOK_LOOP); }
    (?i:break)          { RETURN_TOKEN(TOK_BREAK); }
    (?i:for)            { RETURN_TOKEN(TOK_FOR); }
    (?i:to)             { RETURN_TOKEN(TOK_TO); }
    (?i:step)           { RETURN_TOKEN(TOK_STEP); }
    (?i:next)           { RETURN_TOKEN(TOK_NEXT); }
    (?i:endfunction)    { RETURN_TOKEN(TOK_ENDFUNCTION); }
    (?i:exitfunction)   { RETURN_TOKEN(TOK_EXITFUNCTION); }
    (?i:function)       { RETURN_TOKEN(TOK_FUNCTION); }
    (?i:gosub)          { RETURN_TOKEN(TOK_GOSUB); }
    (?i:return)         { RETURN_TOKEN(TOK_RETURN); }
    (?i:goto)           { RETURN_TOKEN(TOK_GOTO); }
    (?i:dim)            { RETURN_TOKEN(TOK_DIM); }
    (?i:global)         { RETURN_TOKEN(TOK_GLOBAL); }
    (?i:local)          { RETURN_TOKEN(TOK_LOCAL); }
    (?i:as)             { RETURN_TOKEN(TOK_AS); }
    (?i:endtype)        { RETURN_TOKEN(TOK_ENDTYPE); }
    (?i:type)           { RETURN_TOKEN(TOK_TYPE); }
    (?i:boolean)        { RETURN_TOKEN(TOK_BOOLEAN); }
    (?i:integer)        { RETURN_TOKEN(TOK_INTEGER); }
    (?i:dword)          { RETURN_TOKEN(TOK_DWORD); }
    (?i:word)           { RETURN_TOKEN(TOK_WORD); }
    (?i:byte)           { RETURN_TOKEN(TOK_BYTE); }
    (?i:float)          { RETURN_TOKEN(TOK_FLOAT); }
    (?i:double)         { RETURN_TOKEN(TOK_DOUBLE); }
    (?i:string)         { RETURN_TOKEN(TOK_STRING); }
    (?i:endselect)      { RETURN_TOKEN(TOK_ENDSELECT); }
    (?i:select)         { RETURN_TOKEN(TOK_SELECT); }
    (?i:endcase)        { RETURN_TOKEN(TOK_ENDCASE); }
    (?i:case)           { RETURN_TOKEN(TOK_CASE); }
    (?i:default)        { RETURN_TOKEN(TOK_DEFAULT); }

    {SYMBOL}            { yylval->string = odb::str::newCStr(yytext); RETURN_TOKEN(TOK_SYMBOL); }

    "#"                 { RETURN_TOKEN('#'); }
    "$"                 { RETURN_TOKEN('$'); }
    "."                 { RETURN_TOKEN('.'); }

    "\n"                { RETURN_TOKEN('\n'); }
    ":"                 { RETURN_TOKEN(':'); }
    ";"                 { RETURN_TOKEN(';'); }
    [ \t]
    .                   { RETURN_TOKEN(yytext[0]); }
}
%%
