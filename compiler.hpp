#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <iostream>
#include <string>

#include "ast.hpp"
#include "parser.hpp"
#include "table.hpp"
#include "codegen.hpp"

/*
    This file will contain the main logic of the compiler. 
    Lexical -> Syntactic -> Semantic -> Code Generation 
*/

void compile(char *programName,std::string code){

    AST_program *program = parse_program(code);
    program->print();
    std::cout << std::endl;
    SYMBOL_TABLE->printSymbolTable();
    
}


#endif