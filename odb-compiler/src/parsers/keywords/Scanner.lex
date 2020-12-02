%{
    #define YYSTYPE KWSTYPE
    #define YYLTYPE KWLTYPE
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

    #include "odb-compiler/parsers/keywords/Parser.y.h"
    #include "odb-compiler/parsers/keywords/Scanner.hpp"
    #include "odb-util/Str.hpp"

    #if defined(ODBCOMPILER_VERBOSE_FLEX)
    #   define dbg(text) fprintf(stderr, text ": \"%s\"\n", yytext)
    #else
    #   define dbg(text)
    #endif
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="odb::kw::Driver*"
%option prefix="kw"

%%
"="                                   { dbg("delim"); return TOK_EQ; }
"("                                   { dbg("lb"); return TOK_LB; }
")"                                   { dbg("rb"); return TOK_RB; }
"["                                   { dbg("ls"); return TOK_LS; }
"]"                                   { dbg("rs"); return TOK_RS; }
","                                   { dbg("comma"); return TOK_COMMA; }
"|"                                   { dbg("comma"); return TOK_PIPE; }
[\t ]+                                { dbg("whitespace"); }
"\n"                                  { dbg("newline"); return TOK_NEWLINE; }
(?i:"no parameters")                  { dbg("no params"); return TOK_NO_PARAMS; }
[a-zA-Z0-9_/\\$# ]+\.html?            { dbg("help file"); yylval->string = odb::newCStr(yytext); return TOK_HELPFILE; }
[a-zA-Z0-9_$#\- ]+                    { dbg("words"); yylval->string = odb::newCStr(yytext); return TOK_WORDS; }
.                                     {}
%%
