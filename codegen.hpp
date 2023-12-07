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
void GEN_program(AST_program *program, char *programName){
    std::string filename(programName);
    //replace the .ion with .asm
    filename.replace(filename.end()-4, filename.end(), ".asm");

    std::ofstream outFile(filename, std::ios::out);

    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << programName << std::endl;
        return;
    }

    // Writing standard header for assembly file
    outFile << "; Generated Assembly Code for " << programName << std::endl;

    // Generating assembly code for the entire program
    // for (auto expr : program->expressions) {
    //     expr->generateAssembly();
    // }

    // Closing the file
    outFile.close();
}




#endif // CODEGEN_HPP