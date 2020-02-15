%{
    #include "odbc/Parser.y.h"

    #define dbg(text) printf(text ": \"%s\"\n", yytext)
%}

%option nodefault
%option noyywrap
%option bison-bridge
%option reentrant
%option extra-type="odbc::Driver*"

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
FLOAT1          -?[0-9]+\.[0-9]+?
FLOAT2          -?\.[0-9]+
FLOAT3          -?[0-9]+\.[0-9]+?{FLOAT_EXP}?
FLOAT4          -?\.[0-9]+{FLOAT_EXP}?
FLOAT5          -?[0-9]+{FLOAT_EXP}
FLOAT           {FLOAT1}f?|{FLOAT2}f?|{FLOAT3}|{FLOAT4}|{FLOAT5}
INTEGER_BASE2   %[01]+
INTEGER_BASE16  0x[0-9a-fA-F]+
INTEGER         -?[0-9]+
SYMBOL          [a-zA-Z_][a-zA-Z0-9_]+?

%%

{REMARK}            { printf("remark: \"%s\"", yytext); }

{CONSTANT}          { dbg("constant"); return TOK_CONSTANT; }

{BOOL_TRUE}         { dbg("bool"); yylval->boolean_value = true; return TOK_BOOLEAN; }
{BOOL_FALSE}        { dbg("bool"); yylval->boolean_value = false; return TOK_BOOLEAN; }
{STRING_LITERAL}    {
                        dbg("string literal");
                        int len = strlen(yytext);
                        yylval->string_literal = (char*)malloc(len - 2 + 1);
                        memcpy(yylval->string_literal, &yytext[1], len - 2);
                        yylval->string_literal[len - 2] = '\0';
                        return TOK_STRING_LITERAL;
                    }
{FLOAT}             { dbg("float"); yylval->float_value = atof(yytext); return TOK_FLOAT; }
{INTEGER_BASE2}     { dbg("integer"); yylval->integer_value = strtol(&yytext[2], nullptr, 2); return TOK_INTEGER; }
{INTEGER_BASE16}    { dbg("integer"); yylval->integer_value = strtol(&yytext[2], nullptr, 16); return TOK_INTEGER; }
{INTEGER}           { dbg("integer"); yylval->integer_value = strtol(yytext, nullptr, 10); return TOK_INTEGER; }

"+"                 { dbg("add"); return TOK_ADD; }
"-"                 { dbg("sub"); return TOK_SUB; }
"*"                 { dbg("mul"); return TOK_MUL; }
"/"                 { dbg("div"); return TOK_DIV; }
"%"                 { dbg("mod"); return TOK_MOD; }
"^"                 { dbg("pow"); return TOK_POW; }
"("                 { dbg("lb"); return TOK_LB; }
")"                 { dbg("rb"); return TOK_RB; }

"<<"                { dbg("bshl"); return TOK_BSHL; }
">>"                { dbg("bshr"); return TOK_BSHR; }
"||"                { dbg("bor"); return TOK_BOR; }
"&&"                { dbg("band"); return TOK_BAND; }
"~~"                { dbg("bxor"); return TOK_BXOR; }
".."                { dbg("bnot"); return TOK_BNOT; }

"<>"                { dbg("ne"); return TOK_NE; }
"<="                { dbg("le"); return TOK_LE; }
">="                { dbg("ge"); return TOK_GE; }
"="                 { dbg("eq"); return TOK_EQ; }
"<"                 { dbg("lt"); return TOK_LT; }
">"                 { dbg("gt"); return TOK_GT; }
(?i:or)             { dbg("or"); return TOK_OR; }
(?i:and)            { dbg("and"); return TOK_AND; }
(?i:not)            { dbg("not"); return TOK_NOT; }

{SYMBOL}            { dbg("symbol"); yylval->symbol = strdup(yytext); return TOK_SYMBOL; }

[\n:]               { dbg("end statement"); return TOK_END_STATEMENT; }
.                   {}
%%
