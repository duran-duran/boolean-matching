%{
#include "verilog.tab.h"
#include <string.h>
%}

%option outfile="lex.yy.c"
%option yylineno

%%
module                  return MODULE;
endmodule               return ENDMODULE;
input                   return INPUT;
output                  return OUTPUT;
wire                    return WIRE;
and                     return AND;
nand                    return NAND;
or                      return OR;
nor                     return NOR;
xor                     return XOR;
xnor                    return XNOR;
buf                     return BUF;
not                     return NOT;
_cut                    return CUT;
1'b[01]                 yylval.constant = (yytext[3] - '0'); return CONSTANT;
[a-zA-Z_][a-zA-Z0-9_]*  yylval.string = strdup(yytext); return WORD;
\(                      return LEFT_BRACKET;
\)                      return RIGHT_BRACKET;
;                       return SEMICOLON;
,                       return COMMA;
[ \t\n\r]+              for (int i = 0; i < strlen(yytext); ++i) if (yytext[i] == '\n') ++yylineno;
%%

