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

    outFile << "; Generated Assembly Code for " << programName << std::endl;

    // Add the necessary boilerplate code
    outFile << "format pe64 console" << std::endl;
    outFile << "entry start" << std::endl << std::endl;

    outFile << "section '.data' data readable writeable" << std::endl << std::endl;
    outFile << "section '.text' code readable executable" << std::endl;
    outFile << "start:" << std::endl;

    // Your generated assembly code for the program goes here
    // Uncomment the following loop if you have expressions to generate
    // for (auto expr : program->expressions) {
    //     expr->generateAssembly();
    // }

    outFile << std::endl;
    outFile << "section '.idata' import data readable writeable" << std::endl << std::endl;
    outFile << "        dd      0,0,0,RVA kernel_name,RVA kernel_table" << std::endl;
    outFile << "        dd      0,0,0,0,0" << std::endl << std::endl;

    outFile << "kernel_table:" << std::endl;
    outFile << "        ExitProcess     dq RVA _ExitProcess" << std::endl;
    outFile << "        GetStdHandle    dq RVA _GetStdHandle" << std::endl;
    outFile << "        WriteFile       dq RVA _WriteFile" << std::endl;
    outFile << "                        dq 0" << std::endl << std::endl;

    outFile << "kernel_name     db 'KERNEL32.DLL',0" << std::endl;
    outFile << "user_name       db 'USER32.DLL',0" << std::endl << std::endl;

    outFile << "_ExitProcess    db 0,0,'ExitProcess',0" << std::endl;
    outFile << "_GetStdHandle   db 0,0,'GetStdHandle',0" << std::endl;
    outFile << "_WriteFile      db 0,0,'WriteFile',0" << std::endl;
    // Closing the file
    outFile.close();
}




#endif // CODEGEN_HPP