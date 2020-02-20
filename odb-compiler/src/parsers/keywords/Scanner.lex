%{
    #include "odbc/parsers/keywords/Parser.y.h"
    #include "odbc/parsers/keywords/Scanner.hpp"

    #define YYSTYPE KWSTYPE

    #define dbg(text)
    //printf(text ": \"%s\"\n", yytext)
%}

%option nodefault
%option noyywrap
%option bison-bridge
%option reentrant
%option extra-type="odbc::kw::Driver*"
%option prefix="kw"

%%
"="                                   { dbg("delim"); return TOK_EQ; }
"("                                   { dbg("lb"); return TOK_LB; }
")"                                   { dbg("rb"); return TOK_RB; }
"["                                   { dbg("ls"); return TOK_LS; }
"]"                                   { dbg("rs"); return TOK_RS; }
","                                   { dbg("comma"); return TOK_COMMA; }
"|"                                   { dbg("comma"); return TOK_PIPE; }
[\t ]                                 { dbg("whitespace"); }
"\n"                                  { dbg("newline"); return TOK_NEWLINE; }
(?i:"no parameters")                  { dbg("no params"); return TOK_NO_PARAMS; }
[a-zA-Z0-9_/\\$# ]+\.html?            { dbg("help file"); yylval->string = strdup(yytext); return TOK_HELPFILE; }
[a-zA-Z0-9_$#\- ]+                    { dbg("words"); yylval->string = strdup(yytext); return TOK_WORDS; }
.                                     {}
%%
