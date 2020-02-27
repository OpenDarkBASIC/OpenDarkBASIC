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

#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/db/Driver.hpp"

#define driver (static_cast<odbc::db::Driver*>(dbget_extra(yyg)))
#define dbg(text) \
    //printf(text ": \"%s\"\n", yytext)

#define RETURN_TOKEN(token) do {                                              \
        dbg(#token);                                                          \
        return TOK_##token; }                                                 \
    while(0)

#define MAYBE_RETURN_KEYWORD() do {                                           \
        /*                                                                    \
         * This is a hack, but because keywords have spaces in them, it is    \
         * not possible to know where keywords start and where they stop. This  \
         * function looks up the matched symbol in a list of DarkBASIC keywords \
         * (previously loaded and passed to the parser) and tries to expand the \
         * symbol into the full command.                                      \
         */                                                                   \
        bool boundaryOverflow;                                                \
        bool keywordMatched = driver->tryMatchKeyword(yytext, &yy_cp, &yyleng, &yyg->yy_hold_char, &yyg->yy_c_buf_p, &boundaryOverflow); \
        if (boundaryOverflow)                                                 \
        {                                                                     \
            yy_act = YY_END_OF_BUFFER;                                        \
            goto do_action;                                                   \
        }                                                                     \
        if (keywordMatched)                                                   \
        {                                                                     \
            yylval->string = strdup(yytext);                                  \
            dbg("keyword");                                                   \
            return TOK_KEYWORD;                                               \
        }                                                                     \
    } while(0)

#define MAYBE_RETURN_KEYWORD_IF_LONGER() do {                                 \
        int oldTokenLen = strlen(yytext);                                     \
        bool boundaryOverflow;                                                \
        bool keywordMatched = driver->tryMatchKeyword(yytext, &yy_cp, &yyleng, &yyg->yy_hold_char, &yyg->yy_c_buf_p, &boundaryOverflow); \
        if (boundaryOverflow)                                                 \
        {                                                                     \
            yy_act = YY_END_OF_BUFFER;                                        \
            goto do_action;                                                   \
        }                                                                     \
        if (keywordMatched && oldTokenLen < (int)strlen(yytext))              \
        {                                                                     \
            yylval->string = strdup(yytext);                                  \
            dbg("keyword");                                                   \
            return TOK_KEYWORD;                                               \
        }                                                                     \
    } while(0)

static char* strdup_range(const char* src, int beg, int end)
{
    char* result = (char*)malloc(end - beg + 1);
    strncpy(result, src + beg, end - beg);
    result[end - beg] = '\0';
    return result;
}
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="odbc::db::Driver*"
%option prefix="db"

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

%%

{REMARK}            { char* remark = yytext; dbg(remark); (void)remark; }

{CONSTANT}          { RETURN_TOKEN(CONSTANT); }

{BOOL_TRUE}         { yylval->boolean_value = true; return TOK_BOOLEAN_LITERAL; }
{BOOL_FALSE}        { yylval->boolean_value = false; return TOK_BOOLEAN_LITERAL; }
{STRING_LITERAL}    { yylval->string = strdup_range(yytext, 1, strlen(yytext) - 1); return TOK_STRING_LITERAL; }
{FLOAT}             { yylval->float_value = atof(yytext); return TOK_FLOAT_LITERAL; }
{INTEGER_BASE2}     { yylval->integer_value = strtol(&yytext[2], nullptr, 2); return TOK_INTEGER_LITERAL; }
{INTEGER_BASE16}    { yylval->integer_value = strtol(&yytext[2], nullptr, 16); return TOK_INTEGER_LITERAL; }
{INTEGER}           { yylval->integer_value = strtol(yytext, nullptr, 10); return TOK_INTEGER_LITERAL; }

"+"                 { RETURN_TOKEN(ADD); }
"-"                 { RETURN_TOKEN(SUB); }
"*"                 { RETURN_TOKEN(MUL); }
"/"                 { RETURN_TOKEN(DIV); }
"%"                 { RETURN_TOKEN(MOD); }
"^"                 { RETURN_TOKEN(POW); }
"("                 { RETURN_TOKEN(LB); }
")"                 { RETURN_TOKEN(RB); }
","                 { RETURN_TOKEN(COMMA);}
(?i:inc)            { RETURN_TOKEN(INC); }
(?i:dec)            { RETURN_TOKEN(DEC); }

"<<"                { RETURN_TOKEN(BSHL); }
">>"                { RETURN_TOKEN(BSHR); }
"||"                { RETURN_TOKEN(BOR); }
"&&"                { RETURN_TOKEN(BAND); }
"~~"                { RETURN_TOKEN(BXOR); }
".."                { RETURN_TOKEN(BNOT); }

"<>"                { RETURN_TOKEN(NE); }
"<="                { RETURN_TOKEN(LE); }
">="                { RETURN_TOKEN(GE); }
"="                 { RETURN_TOKEN(EQ); }
"<"                 { RETURN_TOKEN(LT); }
">"                 { RETURN_TOKEN(GT); }
(?i:or)             { RETURN_TOKEN(LOR); }
(?i:and)            { RETURN_TOKEN(LAND); }
(?i:not)            { RETURN_TOKEN(LNOT); }

(?:then)            { RETURN_TOKEN(THEN); }
(?:endif)           { RETURN_TOKEN(ENDIF); }
(?:elseif)          { RETURN_TOKEN(ELSEIF); }
(?:if)              { RETURN_TOKEN(IF); }
(?:else)            { RETURN_TOKEN(ELSE); }
(?:endwhile)        { RETURN_TOKEN(ENDWHILE); }
(?:while)           { RETURN_TOKEN(WHILE); }
(?:repeat)          { RETURN_TOKEN(REPEAT); }
(?:until)           { RETURN_TOKEN(UNTIL); }
(?:do)              { RETURN_TOKEN(DO); }
(?:loop)            { MAYBE_RETURN_KEYWORD_IF_LONGER(); RETURN_TOKEN(LOOP); }
(?:for)             { RETURN_TOKEN(FOR); }
(?:to)              { RETURN_TOKEN(TO); }
(?:step)            { RETURN_TOKEN(STEP); }
(?:next)            { RETURN_TOKEN(NEXT); }
(?:endfunction)     { RETURN_TOKEN(ENDFUNCTION); }
(?:exitfunction)    { RETURN_TOKEN(EXITFUNCTION); }
(?:function)        { RETURN_TOKEN(FUNCTION); }
(?:gosub)           { RETURN_TOKEN(GOSUB); }
(?:return)          { RETURN_TOKEN(RETURN); }
(?:dim)             { RETURN_TOKEN(DIM); }
(?:global)          { RETURN_TOKEN(GLOBAL); }
(?:local)           { RETURN_TOKEN(LOCAL); }
(?:as)              { RETURN_TOKEN(AS); }
(?:endtype)         { RETURN_TOKEN(ENDTYPE); }
(?:type)            { RETURN_TOKEN(TYPE); }
(?:boolean)         { RETURN_TOKEN(BOOLEAN); }
(?:integer)         { RETURN_TOKEN(INTEGER); }
(?:float)           { RETURN_TOKEN(FLOAT); }
(?:string)          { RETURN_TOKEN(STRING); }

{SYMBOL}            { MAYBE_RETURN_KEYWORD(); yylval->string = strdup(yytext); RETURN_TOKEN(SYMBOL); }

"#"                 { RETURN_TOKEN(HASH); }
"$"                 { RETURN_TOKEN(DOLLAR); }
"."                 { RETURN_TOKEN(PERIOD); }

"\n"                { RETURN_TOKEN(NEWLINE); }
":"                 { RETURN_TOKEN(COLON); }
.                   {}
%%
