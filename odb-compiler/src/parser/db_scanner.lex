%{
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/utf8.h"

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line;                                   \
    yylloc->first_column = yylloc->last_column;                               \
    for(int i = 0; yytext[i] != '\0'; i++) {                                  \
        if(yytext[i] == '\n') {                                               \
            yylloc->last_line++;                                              \
            yylloc->last_column = 1;                                          \
        } else {                                                              \
            yylloc->last_column++;                                            \
        }                                                                     \
    }

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
%option prefix="db"

REMARK          ((?i:rem)|"`"|"//")
REMARK_START    ((?i:remstart)|"/*")
REMARK_END      ((?i:remend)|"*/")

CONSTANT        #constant

BOOL_TRUE       (?i:true)
BOOL_FALSE      (?i:false)
STRING_LITERAL  \"[^"]*\"
INTEGER_BASE2   %[01]+
INTEGER_BASE16  0[xX][0-9a-fA-F]+
INTEGER         [0-9]+
IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_]+?

%x MULTI_COMMENT
%x MULTI_COMMENT_C
%x SINGLE_COMMENT

%%
<INITIAL>{
    (?i:remstart)       { BEGIN(MULTI_COMMENT); dbg("multiline remark"); }
    "/*"                { BEGIN(MULTI_COMMENT_C); dbg("multiline remark"); }
    ((?i:rem)|"`"|"//") { BEGIN(SINGLE_COMMENT); dbg("single line remark"); }
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

    {BOOL_TRUE}         { yylval->boolean_value = 1; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {BOOL_FALSE}        { yylval->boolean_value = 0; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {STRING_LITERAL}    { yylval->string_value = cstr_utf8_range(yytext); RETURN_TOKEN(TOK_STRING_LITERAL); }
    {INTEGER_BASE2}     { yylval->integer_value = strtol(&yytext[1], NULL, 2); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER_BASE16}    { yylval->integer_value = strtol(&yytext[2], NULL, 16); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER}           { yylval->integer_value = strtol(yytext, NULL, 10); RETURN_TOKEN(TOK_INTEGER_LITERAL); }

    {IDENTIFIER}        { yylval->string_value = cstr_utf8_range(yytext); RETURN_TOKEN(TOK_IDENTIFIER); }

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
