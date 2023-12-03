#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <iostream>
#include <string>
#include "parser.hpp"

/*
    This file will contain the main logic of the compiler. 
    Lexical -> Syntactic -> Semantic -> Code Generation 
*/


void compile(char *programName,std::string code){
    AST_program *program = parse_program(code);
    program->print();
}


#endif