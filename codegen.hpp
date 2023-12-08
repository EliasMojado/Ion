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
#include "table.hpp"

//------------------------------------------------------------------------------------------
// Code Generator
//------------------------------------------------------------------------------------------

// Global variable so all functions can write to the same file
std::ofstream asmFile;

// GENERATE: Program
// - Writes the assembly code for the program
void generate_code(AST_program *program, std::string programName){
    asmFile.open(programName + ".asm");
    if (!asmFile) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    // Writing the boilerplate for an empty FASM program
    asmFile << "format pe64 console\n";
    asmFile << "entry start\n\n";

    asmFile << "STD_OUTPUT_HANDLE       = -11\n\n";

    asmFile << "section '.data' data readable writeable\n";
    asmFile << "    ; Data section goes here\n";
    asmFile << "    dummy db 0  ; Placeholder to keep the section\n\n";

    asmFile << "section '.text' code readable executable\n";
    asmFile << "start:\n";

    // Align scope_size to 16 bytes for stack alignment
    int alignedScopeSize = (SYMBOL_TABLE->scope_size + 15) & ~15;
    asmFile << "    sub rsp, " << alignedScopeSize << "  ; Allocate stack space for program\n";

    for(auto child: program->expressions){
        child->generate_code();
    }

    // Deallocate stack space
    asmFile << "    add rsp, " << alignedScopeSize  << "  ; Deallocate stack space for program\n";

    asmFile << "    mov ecx, 0  ; Exit code\n";
    asmFile << "    call [ExitProcess]\n\n";

    asmFile << "section '.idata' import data readable writeable\n";
    asmFile << "    dd      0,0,0,RVA kernel_name,RVA kernel_table\n";
    asmFile << "    dd      0,0,0,0,0\n\n";

    asmFile << "kernel_table:\n";
    asmFile << "    ExitProcess     dq RVA _ExitProcess\n";
    asmFile << "    dq 0\n\n";

    asmFile << "kernel_name     db 'KERNEL32.DLL',0\n\n";

    asmFile << "_ExitProcess    db 0,0,'ExitProcess',0\n";

    asmFile.close(); // Close the file
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