#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>
#include <unordered_map>
#include <fstream>

#include "ast.hpp"
#include "parser.hpp"
#include "lexer.hpp"

//------------------------------------------------------------------------------------------
// Code Generator
//------------------------------------------------------------------------------------------

// GENERATE: Program
// - Writes the assembly code for the program
void generate_code(AST_program *program, std::string programName){
    for(auto child: program->expressions){
        child->generate_code();
    }
}

void AST_integer::generate_code(){

}

void AST_boolean::generate_code(){
    
}

void AST_float::generate_code(){
    
}

void AST_char::generate_code(){
    
}

void AST_string::generate_code(){
    
}

void AST_variable::generate_code(){
    
}

void AST_unary::generate_code(){
    
}

void AST_binary::generate_code(){
    
}

void AST_block::generate_code(){
    
}

void AST_conditional::generate_code(){
    
}

void AST_loop::generate_code(){
    
}

void AST_function::generate_code(){
    
}

void AST_function_call::generate_code(){
    
}

void AST_return::generate_code(){
    
}


#endif // CODEGEN_HPP