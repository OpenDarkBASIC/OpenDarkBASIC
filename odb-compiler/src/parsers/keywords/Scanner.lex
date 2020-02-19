%{
    #include "odbc/parsers/keywords/Parser.y.h"
    #include "odbc/parsers/keywords/Scanner.hpp"

    #define YYSTYPE KWSTYPE

    #define dbg(text) printf(text ": \"%s\"\n", kwtext)
%}

%option nodefault
%option noyywrap
%option bison-bridge
%option reentrant
%option extra-type="odbc::kw::Driver*"
%option prefix="kw"

%%
.                   {}
%%
