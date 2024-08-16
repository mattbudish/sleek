/*
 * cpp-lexer.lex : A Flex lexer
 *               that finds headers and main functions
 *               ignoring C and C++ comments.
 */

%{
#include <iostream>
#include <fstream>

#include "cppParser.hpp"

using namespace std;
namespace cm = sleek::codeModel;

function<void(const std::string &, cm::IncludeStyle)> headerFileCb;

function<void(void)> mainFoundCb;

%}

%option noyywrap c++

%x STRING IN_COMMENT INCLUDE ANGLE_HEADER QUOTE_HEADER
%%
\"                BEGIN(STRING); 
<STRING>[^\"\n]*
<STRING>\"        BEGIN(INITIAL);

<INITIAL>{
"/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(INITIAL);
[^*\n]+   // eat comment in chunks
"*"       // eat the lone star
\n        yylineno++;
}

<INITIAL>"//".*\n           yylineno++;    // Ignore C++ style comments.

<INITIAL>#include           BEGIN(INCLUDE);
<INCLUDE>[ \t]+             // Ignore white space
<INCLUDE>\"                 BEGIN(QUOTE_HEADER);
<QUOTE_HEADER>[^(\")\n]*    headerFileCb(YYText(), cm::IncludeStyle::Quote);
<QUOTE_HEADER>\"            BEGIN(INITIAL);
<INCLUDE>\<                 BEGIN(ANGLE_HEADER);
<ANGLE_HEADER>[^(\>)\n]*    headerFileCb(YYText(), cm::IncludeStyle::Angle);
<ANGLE_HEADER>\>            BEGIN(INITIAL);

"int main\("            mainFoundCb();

\n                      yylineno++;
.                       // Ignore everything else.

%%

void sleek::codeModel::parseFile(
    const std::string &fileName, 
    std::function<void(const std::string &, cm::IncludeStyle)> header_file_cb, 
    std::function<void(void)> main_found_cb)
{
    ifstream fs(fileName, fstream::in);
    FlexLexer *lexer = new yyFlexLexer(fs, cout);

    headerFileCb = header_file_cb;
    mainFoundCb = main_found_cb;

    while(lexer->yylex() != 0)
        ;

    delete lexer;

    fs.close();
}
