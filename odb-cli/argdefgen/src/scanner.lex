%{
#define YY_USER_ACTION do {                          \
        int i;                                       \
        yylloc->first_line = yylloc->last_line;      \
        yylloc->first_column = yylloc->last_column;  \
        for(i = 0; yytext[i] != '\0'; i++) {         \
            if(yytext[i] == '\n') {                  \
                yylloc->last_line++;                 \
                yylloc->last_column = 1;             \
            }                                        \
            else {                                   \
                yylloc->last_column++;               \
            }                                        \
        }                                            \
    } while (0);

#define drv ((struct adg_driver*)adgget_extra(yyg))

#include "argdefgen/driver.h"
#include "argdefgen/str.h"

static char* section_name(const char* str);
static char* func_name(const char* str);
static char* action_name(const char* str);
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="struct adg_driver*"
%option prefix="adg"

%x HEADER_PREAMBLE_TAG
%x HEADER_POSTAMBLE_TAG
%x SOURCE_PREAMBLE_TAG
%x SOURCE_POSTAMBLE_TAG
%x ACTION_TABLE_TAG

%x HEADER_PREAMBLE
%x HEADER_POSTAMBLE
%x SOURCE_PREAMBLE
%x SOURCE_POSTAMBLE
%x ACTION_TABLE

%x ACTION_TABLE_COMMENT
%x HELP
%x ARGS
%x RUNAFTER
%x REQUIRES

SEC_NAME  [a-zA-Z0-9\-_]+
FUNC_NAME [a-zA-Z0-9_]+
SHORT_OPT [a-zA-Z0-9_]
META_LIST [a-zA-Z0-9\-_, ]+
EXPLICIT_META_ACTION {SEC_NAME}\({SHORT_OPT}?\)\[{META_LIST}\]
IMPLICIT_META_ACTION {SEC_NAME}\[{META_LIST}\]
EXPLICIT_ACTION {SEC_NAME}\({SHORT_OPT}?\)
IMPLICIT_ACTION {SEC_NAME}

%%
<INITIAL>{
    "%header-preamble"               { BEGIN(HEADER_PREAMBLE_TAG); }
    "%header-postamble"              { BEGIN(HEADER_POSTAMBLE_TAG); }
    "%source-preamble"               { BEGIN(SOURCE_PREAMBLE_TAG); }
    "%source-postamble"              { BEGIN(SOURCE_POSTAMBLE_TAG); }
    "%action-table"                  { BEGIN(ACTION_TABLE_TAG); }
    .
    \n
}
<HEADER_PREAMBLE_TAG>{
    "{"                              { BEGIN(HEADER_PREAMBLE); return TOK_HEADER_PREAMBLE_START; }
    [[:blank:]]
    \n
    .                                { return yytext[0]; }
}
<HEADER_POSTAMBLE_TAG>{
    "{"                              { BEGIN(HEADER_POSTAMBLE); return TOK_HEADER_POSTAMBLE_START; }
    [[:blank:]]
    \n
    .                                { return yytext[0]; }
}
<SOURCE_PREAMBLE_TAG>{
    "{"                              { BEGIN(SOURCE_PREAMBLE); return TOK_SOURCE_PREAMBLE_START; }
    [[:blank:]]
    \n
    .                                { return yytext[0]; }
}
<SOURCE_POSTAMBLE_TAG>{
    "{"                              { BEGIN(SOURCE_POSTAMBLE); return TOK_SOURCE_POSTAMBLE_START; }
    [[:blank:]]
    \n
    .                                { return yytext[0]; }
}
<ACTION_TABLE_TAG>{
    "{"                              { BEGIN(ACTION_TABLE); return TOK_ACTION_TABLE_START; }
    [[:blank:]]
    \n
    .                                { return yytext[0]; }
}
<HEADER_PREAMBLE,HEADER_POSTAMBLE,SOURCE_PREAMBLE,SOURCE_POSTAMBLE,ACTION_TABLE>{
    "%}"                             { BEGIN(INITIAL); return TOK_BLOCK_END; }
}
<HEADER_PREAMBLE,HEADER_POSTAMBLE,SOURCE_PREAMBLE,SOURCE_POSTAMBLE>{
    .*                               { yylval->string_value = adg_str_dup(yytext); return TOK_STRING; }
    \n                               { yylval->string_value = adg_str_dup("\n"); return TOK_STRING; }
}
<ACTION_TABLE,HELP>{
    ^" "+?section" "{SEC_NAME}:      { BEGIN(ACTION_TABLE); yylval->string_value = section_name(yytext); return TOK_SECTION; }
    ^" "+?func:" "+?{FUNC_NAME}      { BEGIN(ACTION_TABLE); yylval->string_value = func_name(yytext); return TOK_FUNC; }
    ^" "+?args:" "+?                 { BEGIN(ARGS); return TOK_ARGS; }
    ^" "+?help:" "+?                 { BEGIN(HELP); return TOK_HELP; }
    ^" "+?runafter:" "+?             { BEGIN(RUNAFTER); return TOK_RUNAFTER; }
    ^" "+?requires:" "+?             { BEGIN(REQUIRES); return TOK_REQUIRES; }
    ^" "+?{EXPLICIT_META_ACTION}:    { BEGIN(ACTION_TABLE); yylval->string_value = action_name(yytext); return TOK_EXPLICIT_META_ACTION; }
    ^" "+?{IMPLICIT_META_ACTION}:    { BEGIN(ACTION_TABLE); yylval->string_value = action_name(yytext); return TOK_IMPLICIT_META_ACTION; }
    ^" "+?{EXPLICIT_ACTION}:         { BEGIN(ACTION_TABLE); yylval->string_value = action_name(yytext); return TOK_EXPLICIT_ACTION; }
    ^" "+?{IMPLICIT_ACTION}:         { BEGIN(ACTION_TABLE); yylval->string_value = action_name(yytext); return TOK_IMPLICIT_ACTION; }
}
<ACTION_TABLE>{
    #                                { BEGIN(ACTION_TABLE_COMMENT); }
    \n
    .
}
<HELP>{
    ^" "+?
    .                                { adg_driver_append_help_str(drv, yytext[0]); }
    \n                               { yylval->string_value = adg_driver_take_help_str(drv); return TOK_STRING; }

}
<ACTION_TABLE_COMMENT>{
    \n                               { BEGIN(ACTION_TABLE); }
    .
}
<ARGS>{
    [\[\]<>\|]                       { return yytext[0]; }
    "..."                            { return TOK_ELLIPSIS; }
    [a-zA-Z0-9\-_]+                  { yylval->string_value = adg_str_dup(yytext); return TOK_STRING; }
    \n                               { BEGIN(ACTION_TABLE); }
    .
}
<RUNAFTER,REQUIRES>{
    [a-zA-Z0-9\-]+                   { yylval->string_value = adg_str_dup(yytext); return TOK_STRING; }
    ,                                { return yytext[0]; }
    \n                               { BEGIN(ACTION_TABLE); }
    .
}
%%

static char* section_name(const char* str)
{
    str = strchr(str, ' ');
    do { str++; } while (*str == ' ');
    return adg_str_dup_range(str, 0, strlen(str) - 1);
}
static char* func_name(const char* str)
{
    str = strchr(str, ':');
    do { str++; } while (*str == ' ');
    return adg_str_dup(str);
}
static char* action_name(const char* str)
{
    do { str++; } while (*str == ' ');
    return adg_str_dup_range(str, 0, strlen(str) - 1);
}
