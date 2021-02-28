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
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="struct adg_driver*"
%option prefix="adg"

%x INITIAL_COMMENT
%x HELP_COMMENT
%x HELP
%x ARGS
%x RUNAFTER

SEC_NAME  [a-zA-Z0-9\-_]+
FUNC_NAME [a-zA-Z0-9_]+
SHORT_OPT [a-zA-Z0-9_]+
META_LIST [a-zA-Z0-9, ]+
EXPLICIT_META_ACTION {SEC_NAME}\({SHORT_OPT}?\)\[{META_LIST}\]
IMPLICIT_META_ACTION {SEC_NAME}\[{META_LIST}\]
EXPLICIT_ACTION {SEC_NAME}\({SHORT_OPT}?\)
IMPLICIT_ACTION {SEC_NAME}

%%
<INITIAL,HELP>{
    ^" "+?section" "{SEC_NAME}:" "+?      { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 8, strlen(yytext) - 1); return TOK_SECTION; }
    ^" "+?func:" "+?{FUNC_NAME}           { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 6, strlen(yytext)); return TOK_FUNC; }
    ^" "+?args:" "+?                      { BEGIN(ARGS); return TOK_ARGS; }
    ^" "+?help:" "+?                      { BEGIN(HELP); return TOK_HELP; }
    ^" "+?runafter:" "+?                  { BEGIN(RUNAFTER); return TOK_RUNAFTER; }
    ^" "+?{EXPLICIT_META_ACTION}:" "+?    { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 0, strlen(yytext) - 1); return TOK_EXPLICIT_META_ACTION; }
    ^" "+?{IMPLICIT_META_ACTION}:" "+?    { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 0, strlen(yytext) - 1); return TOK_EXPLICIT_META_ACTION; }
    ^" "+?{EXPLICIT_ACTION}:" "+?         { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 0, strlen(yytext) - 1); return TOK_EXPLICIT_META_ACTION; }
    ^" "+?{IMPLICIT_ACTION}:" "+?         { BEGIN(INITIAL); yylval->string_value = adg_str_dup_range(yytext, 0, strlen(yytext) - 1); return TOK_EXPLICIT_META_ACTION; }
}
<INITIAL>{
    #                                     { BEGIN(INITIAL_COMMENT); }
    \n
    .
}
<HELP>{
    #                                     { BEGIN(HELP_COMMENT); }
    ^" "+?
    .                                     { adg_driver_append_help_str(drv, yytext[0]); }
    \n                                    { yylval->string_value = adg_driver_take_help_str(drv); return TOK_STRING; }

}
<INITIAL_COMMENT>{
    \n                                    { BEGIN(INITIAL); }
    .
}
<HELP_COMMENT>{
    \n                                    { BEGIN(HELP); }
    .
}
<ARGS>{
    [\[\]<>\|]                            { return yytext[0]; }
    "..."                                 { return TOK_ELLIPSIS; }
    [a-zA-Z0-9\-_]+                       { yylval->string_value = adg_str_dup(yytext); return TOK_STRING; }
    \n                                    { BEGIN(INITIAL); }
    .
}
<RUNAFTER>{
    [a-zA-Z0-9\-]+                        { yylval->string_value = adg_str_dup(yytext); return TOK_STRING; }
    ,                                     { return yytext[0]; }
    \n                                    { BEGIN(INITIAL); }
    .
}
%%
