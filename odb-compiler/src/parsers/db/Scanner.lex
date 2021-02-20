%{
#define YYSTYPE DBSTYPE
#define YYLTYPE DBLTYPE

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 1; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

#include "odb-compiler/parsers/db/Parser.y.hpp"
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
    {STRING_LITERAL}    { size_t len = strlen(yytext);
                          yylval->string = odb::str::newCStrRange(yytext, 1, len > 1 ? len-1 : 1); RETURN_TOKEN(TOK_STRING_LITERAL); }
    {FLOAT}             { yylval->float_value = atof(yytext); RETURN_TOKEN(TOK_FLOAT_LITERAL); }
    {INTEGER_BASE2}     { yylval->integer_value = strtol(&yytext[2], nullptr, 2); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER_BASE16}    { yylval->integer_value = strtol(&yytext[2], nullptr, 16); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER}           { yylval->integer_value = strtol(yytext, nullptr, 10); RETURN_TOKEN(TOK_INTEGER_LITERAL); }

    "+"                 { RETURN_TOKEN('+'); }
    "-"                 { RETURN_TOKEN('-'); }
    "*"                 { RETURN_TOKEN('*'); }
    "/"                 { RETURN_TOKEN('/'); }
    "^"                 { RETURN_TOKEN('^'); }
    "("                 { RETURN_TOKEN('('); }
    ")"                 { RETURN_TOKEN(')'); }
    ","                 { RETURN_TOKEN(','); }

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

    {SYMBOL}            { yylval->string = odb::str::newCStr(yytext); RETURN_TOKEN(TOK_SYMBOL); }

    "#"                 { RETURN_TOKEN('#'); }
    "$"                 { RETURN_TOKEN('$'); }
    "%"                 { RETURN_TOKEN('%'); }
    "&"                 { RETURN_TOKEN('&'); }
    "."                 { RETURN_TOKEN('.'); }

    "\n"                { RETURN_TOKEN('\n'); }
    ":"                 { RETURN_TOKEN(':'); }
    ";"                 { RETURN_TOKEN(';'); }
    [ \t\r]
    .                   { RETURN_TOKEN(yytext[0]); }
}
%%
