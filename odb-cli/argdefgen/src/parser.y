%require "3.2"
%code top
{
    #include "argdefgen/parser.y.h"
    #include "argdefgen/scanner.lex.h"
    #include "argdefgen/driver.h"
    #include "argdefgen/action.h"
    #include "argdefgen/str.h"
    #define driver ((struct adg_driver*)adgget_extra(scanner))
    void adgerror(ADGLTYPE* locp, adgscan_t scanner, const char* msg, ...);
}

%code requires
{
    #define YYSTYPE ADGSTYPE
    #define YYLTYPE ADGLTYPE

    #include <stdint.h>
    typedef void* adgscan_t;
    struct adg_driver;
}

%define api.pure full
%define api.push-pull push
%define api.prefix {adg}
%define api.header.include {"argdefgen/parser.y.h"}
%define api.token.prefix {TOK_}

%lex-param {adgscan_t scanner}
%parse-param {adgscan_t scanner}

%locations
%define parse.error verbose

%union {
    char* string_value;
    struct adg_action* node;
}

%destructor { adg_str_free($$); } <string_value>
%destructor { adg_action_destroy($$); } <node>

%token EOF 0 "end of file"
%token ERROR 1 "lexer error"
%token HELP
%token ARGS
%token RUNAFTER
%token ELLIPSIS
%token<string_value> FUNC
%token<string_value> SECTION
%token<string_value> EXPLICIT_ACTION
%token<string_value> IMPLICIT_ACTION
%token<string_value> EXPLICIT_META_ACTION
%token<string_value> IMPLICIT_META_ACTION
%token<string_value> STRING

%%
root
  : sections
  | EOF
  ;
sections
  : sections section
  | section
  ;
section
  : SECTION actions { adg_str_free($1); }
  ;
actions
  : actions action
  | action
  ;
action
  : HELP help_str
  | ARGS parse_args
  | FUNC { adg_str_free($1); }
  | RUNAFTER runafter_list
  | EXPLICIT_ACTION { adg_str_free($1); }
  | IMPLICIT_ACTION { adg_str_free($1); }
  | EXPLICIT_META_ACTION { adg_str_free($1); }
  | IMPLICIT_META_ACTION { adg_str_free($1); }
  ;
help_str
  : help_str STRING { adg_str_free($2); }
  | STRING { adg_str_free($1); }
  ;
parse_args
  : parse_required_args
  ;
parse_required_args
  : parse_required_args '<' required_argnames '>' parse_optional_args
  | '<' required_argnames '>' parse_optional_args
  | parse_required_args '<' required_argnames '>'
  | '<' required_argnames '>'
  | parse_optional_args
  ;
parse_optional_args
  : parse_optional_args '[' optional_argnames ']'
  | '[' optional_argnames ELLIPSIS ']'
  | '[' optional_argnames ']'
  ;
required_argnames
  : STRING '|' required_argnames { adg_str_free($1); }
  | STRING { adg_str_free($1); }
  ;
optional_argnames
  : STRING '|' optional_argnames { adg_str_free($1); }
  | STRING { adg_str_free($1); }
  ;
runafter_list
  : STRING ',' runafter_list { adg_str_free($1); }
  | STRING { adg_str_free($1); }
  ;
%%
