%{
#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/utf8.h"

#define YY_USER_ACTION                      \
    yylloc->off += yylloc->len;             \
    yylloc->len = (utf8_idx)strlen(yytext);

#if defined(ODBCOMPILER_VERBOSE_FLEX)
#   define dbg(text) fprintf(stderr, text ": \"%s\"\n", yytext)
#else
#   define dbg(text)
#endif

#define RETURN_TOKEN(token) do {                                              \
        dbg(#token);                                                          \
        return token;                                                         \
    } while(0)

static inline struct utf8_span
token_to_ref(const char* cstr, void* extra)
{
    char* base = extra;
    struct utf8_span ref = {
        (utf8_idx)(cstr - base),
        (utf8_idx)strlen(cstr)
    };
    return ref;
}
static inline struct utf8_span
token_to_ref_strip_quotes(const char* cstr, void* extra)
{
    char* base = extra;
    struct utf8_span ref = {
        (utf8_idx)(cstr - base) + 1,
        (utf8_idx)strlen(cstr) - 2
    };
    return ref;
}

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
DOUBLE_EXP      [eE][\+-]?[0-9]+
DOUBLE1         [0-9]+\.[0-9]+?
DOUBLE2         \.[0-9]+
DOUBLE3         [0-9]+\.[0-9]+?{DOUBLE_EXP}?
DOUBLE4         \.[0-9]+{DOUBLE_EXP}?
DOUBLE5         [0-9]+{DOUBLE_EXP}
DOUBLE          {DOUBLE1}|{DOUBLE2}|{DOUBLE3}|{DOUBLE4}|{DOUBLE5}
FLOAT           {DOUBLE}[fF]|{INTEGER}[fF]
INTEGER_BASE2   %[01]+
INTEGER_BASE16  0[xX][0-9a-fA-F]+
INTEGER         [0-9]+
IMAG            {DOUBLE}[iIjJkK]|{INTEGER_BASE2}[iIjJkK]|{INTEGER_BASE16}[iIjJkK]|{INTEGER}[iIjJkK]
IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_]+?

%x MULTI_COMMENT
%x MULTI_COMMENT_C
%x SINGLE_COMMENT

%%
<INITIAL>{
    (?i:remstart)       { BEGIN(MULTI_COMMENT); RETURN_TOKEN(TOK_REMSTART); }
    "/*"                { BEGIN(MULTI_COMMENT_C); RETURN_TOKEN(TOK_REMSTART); }
    ((?i:rem)|"`"|"//") { BEGIN(SINGLE_COMMENT); RETURN_TOKEN(TOK_REMSTART); }
}
<SINGLE_COMMENT>{
    .
    \n                  { BEGIN(INITIAL); RETURN_TOKEN(TOK_REMEND); }
}
<MULTI_COMMENT>{
    (?i:remend)         { BEGIN(INITIAL); RETURN_TOKEN(TOK_REMEND); }
    .
    \n
}
<MULTI_COMMENT_C>{
    "*/"                { BEGIN(INITIAL); RETURN_TOKEN(TOK_REMEND); }
    .
    \n
}
<INITIAL>{
    {CONSTANT}          { RETURN_TOKEN(TOK_CONSTANT); }

    {BOOL_TRUE}         { yylval->boolean_value = 1; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {BOOL_FALSE}        { yylval->boolean_value = 0; RETURN_TOKEN(TOK_BOOLEAN_LITERAL); }
    {STRING_LITERAL}    { yylval->string_value = token_to_ref_strip_quotes(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_STRING_LITERAL); }
    {FLOAT}             { yylval->float_value = (float)atof(yytext); RETURN_TOKEN(TOK_FLOAT_LITERAL); }
    {DOUBLE}            { yylval->double_value = atof(yytext); RETURN_TOKEN(TOK_DOUBLE_LITERAL); }
    {INTEGER_BASE2}     { yylval->integer_value = strtoll(&yytext[1], NULL, 2); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER_BASE16}    { yylval->integer_value = strtoll(&yytext[2], NULL, 16); RETURN_TOKEN(TOK_INTEGER_LITERAL); }
    {INTEGER}           { yylval->integer_value = strtoll(yytext, NULL, 10); RETURN_TOKEN(TOK_INTEGER_LITERAL); }

    {IDENTIFIER}"&"       { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER_DOUBLE_INTEGER); }
    {IDENTIFIER}"%"       { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER_WORD); }
    {IDENTIFIER}"!"      { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER_DOUBLE); }
    {IDENTIFIER}"#"     { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER_FLOAT); }
    {IDENTIFIER}"$"     { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER_STRING); }
    {IDENTIFIER}        { yylval->string_value = token_to_ref(yytext, yyget_extra(yyg)); RETURN_TOKEN(TOK_IDENTIFIER); }

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
