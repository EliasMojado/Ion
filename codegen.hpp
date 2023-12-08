#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <set>

#include "ast.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "table.hpp"

//------------------------------------------------------------------------------------------
// Code Generator
//------------------------------------------------------------------------------------------

// REGISTER MANAGER
// - Keeps track of free registers to use
class RegisterManager {
private:
    std::set<std::string> freeRegisters;

public:
    RegisterManager() {
        // Initialize with all available registers
        freeRegisters = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    }

    std::string getFreeRegister() {
        if (freeRegisters.empty()) {
            throw std::runtime_error("No free registers available");
        }
        std::string reg = *freeRegisters.begin();
        freeRegisters.erase(freeRegisters.begin());
        return reg;
    }

    void releaseRegister(const std::string& reg) {
        freeRegisters.insert(reg);
    }
};

// Global variables declaration
std::ofstream asmFile;
RegisterManager regManager; 

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
    asmFile << "    sub rsp, " << alignedScopeSize << "  ; Allocate stack space for program. Size: " << SYMBOL_TABLE->scope_size << "\n";

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

codeGenResult AST_integer::generate_code(){
    std::string reg = regManager.getFreeRegister();
    asmFile << "    mov " << reg << ", " << this->value << "\n";
    codeGenResult res;
    res.registerName = reg;
    res.type = res_type::INTEGER;
    return res;
}

codeGenResult AST_boolean::generate_code(){
    
}

codeGenResult AST_float::generate_code(){
    
}

codeGenResult AST_char::generate_code(){
    
}

codeGenResult AST_string::generate_code(){
    
}

codeGenResult AST_variable::generate_code(){
    
}

codeGenResult AST_unary::generate_code(){
    
}

codeGenResult AST_binary::generate_code(){
    // Generate code for LHS and RHS, and get the registers they use
    codeGenResult lhsReg = LHS->generate_code();
    codeGenResult rhsReg = RHS->generate_code();

    // Check the operation and perform it
    if (op == "+") {
        if(lhsReg.type != res_type::INTEGER || rhsReg.type != res_type::INTEGER){
            throw std::runtime_error("Unsupported operation + on non-integer types");
        }
        asmFile << "    add " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
    } else if (op == "-") {
        if(lhsReg.type != res_type::INTEGER || rhsReg.type != res_type::INTEGER){
            throw std::runtime_error("Unsupported operation - on non-integer types");
        }
        asmFile << "    sub " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
    } else if (op == "*"){
        if(lhsReg.type != res_type::INTEGER || rhsReg.type != res_type::INTEGER){
            throw std::runtime_error("Unsupported operation * on non-integer types");
        }
        asmFile << "    imul " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
    } else if (op == "/"){
        if(lhsReg.type != res_type::INTEGER || rhsReg.type != res_type::INTEGER){
            throw std::runtime_error("Unsupported operation / on non-integer types");
        }
        asmFile << "    mov rax, " << lhsReg.registerName << "\n";
        asmFile << "    cqo\n";
        asmFile << "    idiv " << rhsReg.registerName << "\n";
        asmFile << "    mov " << lhsReg.registerName << ", rax\n";
    } else if (op == "%"){
        if(lhsReg.type != res_type::INTEGER || rhsReg.type != res_type::INTEGER){
            throw std::runtime_error("Unsupported operation modulo on non-integer types");
        }
        asmFile << "    mov rax, " << lhsReg.registerName << "\n";
        asmFile << "    cqo\n";
        asmFile << "    idiv " << rhsReg.registerName << "\n";
        asmFile << "    mov " << lhsReg.registerName << ", rdx\n";
    }

    // Release the RHS register as it's no longer needed
    regManager.releaseRegister(rhsReg.registerName);

    // Return the register holding the result (usually lhsReg)

    return lhsReg;
}

codeGenResult AST_block::generate_code(){
    SYMBOL_TABLE = SYMBOL_TABLE->traverseIN();

    // ALLOCATE STACK SPACE FOR BLOCK
    int alignedScopeSize = (SYMBOL_TABLE->scope_size + 15) & ~15;
    asmFile << "    sub rsp, " << alignedScopeSize << "  ; Allocate stack space for block. Size: " << SYMBOL_TABLE->scope_size << "\n";

    for(auto child: this->children){
        child->generate_code();
    }

    // DEALLOCATE STACK SPACE FOR BLOCK
    asmFile << "    add rsp, " << alignedScopeSize  << "  ; Deallocate stack space for block\n";

    SYMBOL_TABLE = SYMBOL_TABLE->traverseOUT();

    codeGenResult res;
    res.type = res_type::VOID;
    return res;
}

codeGenResult AST_conditional::generate_code(){
    
}

codeGenResult AST_loop::generate_code(){
    
}

codeGenResult AST_function::generate_code(){
    
}

codeGenResult AST_function_call::generate_code(){
    
}

codeGenResult AST_return::generate_code(){
    
}


#endif // CODEGEN_HPP