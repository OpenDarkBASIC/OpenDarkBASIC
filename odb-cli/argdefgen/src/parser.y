%require "3.2"
%code top
{
    #include "argdefgen/parser.y.h"
    #include "argdefgen/scanner.lex.h"
    #include "argdefgen/driver.h"
    #include "argdefgen/node.h"
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
    union adg_node* node_value;
}

%destructor { adg_str_free($$); } <string_value>
%destructor { adg_node_destroy($$); } <node_value>

%token EOF 0 "end of file"
%token ERROR 1 "lexer error"
%token HELP
%token ARGS
%token RUNAFTER
%token REQUIRES
%token ELLIPSIS
%token<string_value> FUNC
%token<string_value> SECTION
%token<string_value> EXPLICIT_ACTION
%token<string_value> IMPLICIT_ACTION
%token<string_value> EXPLICIT_META_ACTION
%token<string_value> IMPLICIT_META_ACTION
%token<string_value> STRING

%type<string_value> help_str
%type<node_value> sections
%type<node_value> section
%type<node_value> actions
%type<node_value> action
%type<node_value> actionattrs
%type<node_value> actionattr
%type<node_value> parse_required_args
%type<node_value> parse_optional_args
%type<node_value> required_argnames
%type<node_value> optional_argnames
%type<node_value> runafter_list
%type<node_value> requires_list

%%
root
  : sections                                      { adg_driver_give(driver, $1); }
  ;
sections
  : sections section                              { $$ = $1; adg_node_append_section($$, $2); }
  | section                                       { $$ = $1; }
  ;
section
  : SECTION actions                               { $$ = adg_node_new_section($2, $1, &@$); }
  ;
actions
  : action actions                                { $$ = $2; adg_node_append_action($$, $1); }
  | action                                        { $$ = $1; }
  ;
action
  : EXPLICIT_ACTION actionattrs                   { $$ = adg_node_new_explicit_action($1, $2, &@$); }
  | IMPLICIT_ACTION actionattrs                   { $$ = adg_node_new_implicit_action($1, $2, &@$); }
  | EXPLICIT_META_ACTION actionattrs              { $$ = adg_node_new_explicit_meta_action($1, $2, &@$); }
  | IMPLICIT_META_ACTION actionattrs              { $$ = adg_node_new_implicit_meta_action($1, $2, &@$); }
  ;
actionattrs
  : actionattrs actionattr                        { $$ = $1; adg_node_append_actionattr($$, $2); }
  | actionattr                                    { $$ = $1; }
  ;
actionattr
  : HELP help_str                                 { $$ = adg_node_new_actionattr(adg_node_new_help($2, &@2), &@$); }
  | ARGS parse_required_args                      { $$ = adg_node_new_actionattr($2, &@$); }
  | FUNC                                          { $$ = adg_node_new_actionattr(adg_node_new_func($1, &@$), &@$); }
  | RUNAFTER runafter_list                        { $$ = adg_node_new_actionattr($2, &@$); }
  | REQUIRES requires_list                        { $$ = adg_node_new_actionattr($2, &@$); }
  ;
help_str
  : help_str STRING                               { $$ = adg_str_join($1, $2, " "); adg_str_free($2); }
  | STRING                                        { $$ = $1; }
  ;
parse_required_args
  : '<' required_argnames '>' parse_optional_args { $$ = adg_node_new_arg($4, $2, &@$); }
  | '<' required_argnames '>'                     { $$ = adg_node_new_arg(NULL, $2, &@$); }
  | parse_optional_args                           { $$ = $1; }
  ;
parse_optional_args
  : '[' optional_argnames ']' parse_optional_args { $$ = adg_node_new_optional_arg($4, $2, 0, &@$); }
  | '[' optional_argnames ELLIPSIS ']'            { $$ = adg_node_new_optional_arg(NULL, $2, 1, &@$); }
  | '[' optional_argnames ']'                     { $$ = adg_node_new_optional_arg(NULL, $2, 0, &@$); }
  ;
required_argnames
  : STRING '|' required_argnames                  { $$ = adg_node_new_argname($3, $1, &@$); }
  | STRING                                        { $$ = adg_node_new_argname(NULL, $1, &@$); }
  ;
optional_argnames
  : STRING '|' optional_argnames                  { $$ = adg_node_new_argname($3, $1, &@$); }
  | STRING                                        { $$ = adg_node_new_argname(NULL, $1, &@$); }
  ;
runafter_list
  : STRING ',' runafter_list                      { $$ = adg_node_new_runafter($3, $1, &@$); }
  | STRING                                        { $$ = adg_node_new_runafter(NULL, $1, &@$); }
  ;
requires_list
  : STRING ',' requires_list                      { $$ = adg_node_new_requires($3, $1, &@$); }
  | STRING                                        { $$ = adg_node_new_requires(NULL, $1, &@$); }
  ;
%%
