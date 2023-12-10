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
        if(reg.empty()){
            return;
        }
        freeRegisters.insert(reg);
    }
};

// Global variables declaration
std::ofstream asmFile;
RegisterManager regManager; 
int GLOBAL_ADDRESS = 0;

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
    asmFile << "include 'win64ax.inc'\n";
    asmFile << "entry start\n\n";

    asmFile << "STD_OUTPUT_HANDLE       = -11\n\n";

    asmFile << "section '.data' data readable writeable\n";
    asmFile << "    ; Data section goes here\n";
    asmFile << "    intFormat db '%d', 0  ; Format string for integers\n\n";
    asmFile << "    buffer rb 64\n\n";
    asmFile << "    intstore rq 0\n";
    asmFile << "    charstore db ' '\n";
    asmFile << "    boolstore rb 1\n";
    asmFile << "    stringstore db ' ', 0\n\n";

    // Write all of the string literals
    for (const auto& pair : stringLiterals) {
        // Write the string literal with its label
        asmFile << "    " << pair.second << " db \"" << pair.first << "\", 0" << std::endl;

        // Write the length of the string literal
        asmFile << "    " << pair.second << "_len = $ - " << pair.second << std::endl;
    }

    asmFile << "section '.text' code readable executable\n";
    asmFile << "start:\n";

    asmFile << "    mov rbp, rsp    ; Set base pointer to the current stack pointer\n";

    // Align scope_size to 16 bytes for stack alignment
    int alignedScopeSize = (SYMBOL_TABLE->scope_size + 15) & ~15;
    asmFile << "    sub rsp, " << alignedScopeSize << "  ; Allocate stack space for program. Size: " << SYMBOL_TABLE->scope_size << "\n";
    GLOBAL_ADDRESS += alignedScopeSize;

    for(auto child: program->expressions){
        codeGenResult res = child->generate_code();
        // if(child->type == AST_type::BOOLEAN ||
        //    child->type == AST_type::INTEGER ||
        //    child->type == AST_type::CHAR ||
        //    child->type == AST_type::STRING ||
        //    child->type == AST_type::FLOAT ||
        //    child->type == AST_type::FUNCTION_CALL){
        //         regManager.releaseRegister(res.registerName);
        //    }

        regManager.releaseRegister(res.registerName);
    }

    // Deallocate stack space
    asmFile << "    add rsp, " << alignedScopeSize  << "  ; Deallocate stack space for program\n";
    GLOBAL_ADDRESS -= alignedScopeSize;

    asmFile << "    mov ecx, 0  ; Exit code\n";
    asmFile << "    call [ExitProcess]\n\n";

    asmFile << "section '.idata' import data readable writeable\n";
    asmFile << "library msvcrt, 'msvcrt.dll', kernel32, 'kernel32.dll'\n";
    asmFile << "import msvcrt, printf, 'printf', scanf, 'scanf', getch, '_getch', sprintf, 'sprintf'\n";
    asmFile << "    dd      0,0,0,RVA kernel_name,RVA kernel_table\n";
    asmFile << "    dd      0,0,0,0,0\n\n";

    asmFile << "kernel_table:\n";
    asmFile << "    ExitProcess     dq RVA _ExitProcess\n";
    asmFile << "    dq 0\n\n";

    asmFile << "kernel_name     db 'KERNEL32.DLL',0\n\n";

    asmFile << "_ExitProcess    db 0,0,'ExitProcess',0\n";

    asmFile.close(); // Close the file
}

codeGenResult CALL_write(AST_function_call *call){
    for (auto& param : call->parameters) {
        if(param->type == AST_type::STRING){
            // Find the label
            std::string label = stringLiterals.find(dynamic_cast<AST_string*>(param)->value)->second;
            asmFile << "    cinvoke printf, " << label << std::endl;
        }else if(param->type == AST_type::INTEGER){
            AST_integer *integer = dynamic_cast<AST_integer*>(param);
            asmFile << "    cinvoke printf, \"%i\", " << integer->value << std::endl;
        }else if(param->type == AST_type::CHAR){
            AST_char *character = dynamic_cast<AST_char*>(param);
            asmFile << "    cinvoke printf, \"%c\", " << character->value << std::endl;
        }else if(param->type == AST_type::BOOLEAN){
            AST_boolean *boolean = dynamic_cast<AST_boolean*>(param);
            if(boolean->value == true)
                asmFile << "    cinvoke printf, \"1\"" << std::endl;
            else if(boolean->value == false)
                asmFile << "    cinvoke printf, \"0\"" << std::endl;
        }else if(param->type == AST_type::FLOAT){
            AST_float *floating = dynamic_cast<AST_float*>(param);
            asmFile << "    cinvoke printf, \"%f\", " << floating->value << std::endl;
        }else if(param->type == AST_type::VARIABLE){
            codeGenResult varResult = param->generate_code();
            switch (varResult.type) {
                case res_type::VAR_INTEGER:
                    asmFile << "    cinvoke sprintf, buffer, \"%d\", " << varResult.registerName << std::endl;  // Format the integer into the buffer
                    asmFile << "    cinvoke printf, buffer" << std::endl;  // Print the formatted string
                    break;
                case res_type::VAR_BOOLEAN:
                    asmFile << "    cinvoke sprintf, buffer, \"%d\", " << varResult.registerName << std::endl;  // Format the integer into the buffer
                    asmFile << "    cinvoke printf, buffer" << std::endl;  // Print the formatted string
                    break;
                case res_type::VAR_CHAR:
                    asmFile << "    cinvoke sprintf, buffer, \"%c\", " << varResult.registerName << std::endl;  // Format the integer into the buffer
                    asmFile << "    cinvoke printf, buffer" << std::endl;  // Print the formatted string
                    break;
                case res_type::VAR_STRING:
                    asmFile << "    cinvoke printf, " << varResult.registerName << std::endl;  // Print the formatted string
                    break;
                default:
                    throw std::runtime_error("Unsupported variable type for write function");
            }

            // Release the register after use
            regManager.releaseRegister(varResult.registerName);
        }else{
            throw std::runtime_error("Unsupported parameter type for write function");
        }
    }

    codeGenResult res;
    res.type = res_type::VOID; // write function doesn't return a value
    return res;
}

codeGenResult CALL_read(AST_function_call *call) {
    for (auto& param : call->parameters) {
        if (param->type == AST_type::VARIABLE) {
            codeGenResult varResult = param->generate_code();

            // Assuming the variable type is specified by the variable itself
            data_type expectedType = SYMBOL_TABLE->getVariable(dynamic_cast<AST_variable*>(param)->name).type;

            switch (expectedType) {
                case data_type::INTEGER:
                    asmFile << "    cinvoke scanf, \"%d\", intstore" << std::endl;
                    asmFile << "    mov " << varResult.registerName << ", [intstore]" << std::endl;
                    asmFile << "    mov [rbp - " << varResult.trueAd << "], " << varResult.registerName << std::endl;
                    break;

                case data_type::CHAR:
                    asmFile << "    cinvoke scanf, \"%c\", charstore" << std::endl;
                    asmFile << "    mov " << varResult.registerName << "b, [charstore]" << std::endl;
                    asmFile << "    mov [rbp - " << varResult.trueAd << "], " << varResult.registerName << std::endl;
                    break;

                case data_type::BOOLEAN:// still needs more refinement on this part: problem on storing boolean value and writing it afterwards
                    asmFile << "    cinvoke scanf, \"%d\", boolstore" << std::endl;
                    asmFile << "    movzx " << varResult.registerName << ", byte [boolstore]" << std::endl;
                    asmFile << "    mov [rbp - " << varResult.trueAd << "], " << varResult.registerName << std::endl;
                    break;

                case data_type::STRING:
                    asmFile << "    cinvoke scanf, \"%s\", stringstore" << std::endl;
                    asmFile << "    lea " << varResult.registerName << ", [stringstore]" << std::endl;
                    asmFile << "    mov [rbp - " << varResult.trueAd << "], " << varResult.registerName << std::endl;
                    break;

                case data_type::UNKNOWN:
                    throw std::runtime_error("Cannot read into a variable of unknown type");

                // Add cases for other variable types as needed

                default:
                    throw std::runtime_error("Unsupported variable type for read function");
            }

            // Release the register after use
            regManager.releaseRegister(varResult.registerName);
        } else {
            throw std::runtime_error("Unsupported parameter type for read function");
        }
    }

    codeGenResult res;
    res.type = res_type::VOID; // read function doesn't return a value
    return res;
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
    std::string reg = regManager.getFreeRegister();
    if(this->value == true){
        asmFile << "    mov " << reg << ", 1\n";
    }else{
        asmFile << "    mov " << reg << ", 0\n";
    }

    codeGenResult res;
    res.registerName = reg;
    res.type = res_type::BOOLEAN;
    return res;
}

codeGenResult AST_float::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Float not implemented yet");
    return res;
}

codeGenResult AST_char::generate_code(){
    std::string reg = regManager.getFreeRegister();
    asmFile << "    mov " << reg << ", '" << this->value << "'\n";
    codeGenResult res; 
    res.registerName = reg;
    res.type = res_type::CHAR;
    return res; 
}

codeGenResult AST_string::generate_code(){
    codeGenResult res;
    auto it = stringLiterals.find(this->value);
    if(it != stringLiterals.end()){
        res.type = res_type::STRING;
        res.registerName = it->second;
    }else{
        throw std::runtime_error("String literal not found in stringLiterals map");
    }

    return res;
}

codeGenResult AST_variable::generate_code(){
    metadata data = SYMBOL_TABLE->getVariable(this->name);
    std::string reg = regManager.getFreeRegister();
    int trueAddress;
    if(data.relative_address == -1){ // Declaration
        trueAddress = GLOBAL_ADDRESS - (data.address + data.size);
        SYMBOL_TABLE->set_relativeAddress(this->name, trueAddress);

        // Calculate the variable's address and load its value into the register
        asmFile << "    mov " << reg << ", [rbp - " << trueAddress << "]" << "; Declare variable: " << this->name << std::endl;
    }else{
        trueAddress = data.relative_address;

        // Calculate the variable's address and load its value into the register
        asmFile << "    mov " << reg << ", [rbp - " << trueAddress << "]" << "; Use variable: " << this->name << std::endl;
    }

    codeGenResult res;
    res.registerName = reg;
    res.trueAd = trueAddress;
    if(data.type == data_type::INTEGER){
        res.type = res_type::VAR_INTEGER;
    }else if(data.type == data_type::BOOLEAN){
        res.type = res_type::VAR_BOOLEAN;
    }else if(data.type == data_type::CHAR){
        res.type = res_type::VAR_CHAR;
    }else if(data.type == data_type::STRING){
        res.type = res_type::VAR_STRING;
    }else if(data.type == data_type::FLOAT){
        res.type = res_type::VAR_FLOAT;
    }else if(data.type == data_type::UNKNOWN){
        res.type = res_type::VAR_UNKNOWN;
    }

    return res;
}

codeGenResult AST_unary::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Unary not implemented yet");
    return res;
}

codeGenResult AST_binary::generate_code(){
    // Generate code for LHS and RHS, and get the registers they use
    codeGenResult lhsReg = LHS->generate_code();
    codeGenResult rhsReg = RHS->generate_code();

    // Check the operation and perform it
    if (op == "+") {
        if((lhsReg.type == res_type::INTEGER || lhsReg.type == res_type::VAR_INTEGER) && 
            (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER)
        ){
            asmFile << "    add " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
        }else{
            throw std::runtime_error("Unsupported operation + on non-integer types");
        }
    } else if (op == "-") {
        if((lhsReg.type == res_type::INTEGER || lhsReg.type == res_type::VAR_INTEGER) && 
            (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER)
        ){
            asmFile << "    sub " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
        }else{
            throw std::runtime_error("Unsupported operation - on non-integer types");
        }
    } else if (op == "*"){
        if((lhsReg.type == res_type::INTEGER || lhsReg.type == res_type::VAR_INTEGER) && 
            (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER)
        ){
            asmFile << "    imul " << lhsReg.registerName << ", " << rhsReg.registerName << "\n";
        }else{
            throw std::runtime_error("Unsupported operation * on non-integer types");
        }
    } else if (op == "/"){
        if((lhsReg.type == res_type::INTEGER || lhsReg.type == res_type::VAR_INTEGER) && 
            (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER)
        ){
            asmFile << "    mov rax, " << lhsReg.registerName << "\n";
            asmFile << "    cqo\n";
            asmFile << "    idiv " << rhsReg.registerName << "\n";
            asmFile << "    mov " << lhsReg.registerName << ", rax\n";
        }else{
            throw std::runtime_error("Unsupported operation / on non-integer types");
        }
    } else if (op == "%"){
        if((lhsReg.type == res_type::INTEGER || lhsReg.type == res_type::VAR_INTEGER) && 
            (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER)
        ){
            asmFile << "    mov rax, " << lhsReg.registerName << "\n";
            asmFile << "    cqo\n";
            asmFile << "    idiv " << rhsReg.registerName << "\n";
            asmFile << "    mov " << lhsReg.registerName << ", rdx\n";
        }else{
            throw std::runtime_error("Unsupported operation % on non-integer types");
        }
    } else if (op == "="){
         // Ensure LHS is a variable
        if (LHS->type != AST_type::VARIABLE) {
            throw std::runtime_error("Left-hand side of assignment must be a variable");
        }

        // Ensure LHS and RHS are the same type
        if((lhsReg.type == res_type::VAR_INTEGER && (rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER))||
            (lhsReg.type == res_type::VAR_BOOLEAN && (rhsReg.type == res_type::BOOLEAN || rhsReg.type == res_type::VAR_BOOLEAN))||
            (lhsReg.type == res_type::VAR_CHAR && (rhsReg.type == res_type::CHAR || rhsReg.type == res_type::VAR_CHAR))||
            (lhsReg.type == res_type::VAR_STRING && (rhsReg.type == res_type::STRING || rhsReg.type == res_type::VAR_STRING))||
            (lhsReg.type == res_type::VAR_FLOAT && (rhsReg.type == res_type::FLOAT || rhsReg.type == res_type::VAR_FLOAT))
        ){
            // Do nothing
        }else if(lhsReg.type == res_type::VAR_UNKNOWN){
            AST_variable* var = dynamic_cast<AST_variable*>(LHS);
            metadata data = SYMBOL_TABLE->getVariable(var->name);
            if(rhsReg.type == res_type::INTEGER || rhsReg.type == res_type::VAR_INTEGER){
                SYMBOL_TABLE->changeType(var->name, data_type::INTEGER);
            }else if(rhsReg.type == res_type::BOOLEAN || rhsReg.type == res_type::VAR_BOOLEAN){
                SYMBOL_TABLE->changeType(var->name, data_type::BOOLEAN);
            }else if(rhsReg.type == res_type::CHAR || rhsReg.type == res_type::VAR_CHAR){
                SYMBOL_TABLE->changeType(var->name, data_type::CHAR);
            }else if(rhsReg.type == res_type::STRING || rhsReg.type == res_type::VAR_STRING){
                SYMBOL_TABLE->changeType(var->name, data_type::STRING);
            }else if(rhsReg.type == res_type::FLOAT || rhsReg.type == res_type::VAR_FLOAT){
                SYMBOL_TABLE->changeType(var->name, data_type::FLOAT);
            }
        }else{
            throw std::runtime_error("Unsupported operation = on non-matching types");
        }

        // Store the RHS value into the LHS variable's location
        asmFile << "    mov " << lhsReg.registerName << ", " << rhsReg.registerName << std::endl;

        // Store the LHS value into the variable's location
        metadata data = SYMBOL_TABLE->getVariable(dynamic_cast<AST_variable*>(LHS)->name);
        asmFile << "    mov [rbp - " << data.relative_address << "], " << lhsReg.registerName << "; store to lhs"<< std::endl;
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
    GLOBAL_ADDRESS += alignedScopeSize;

    for(auto child: this->children){
        child->generate_code();
    }

    // DEALLOCATE STACK SPACE FOR BLOCK
    asmFile << "    add rsp, " << alignedScopeSize  << "  ; Deallocate stack space for block\n";
    GLOBAL_ADDRESS -= alignedScopeSize;

    SYMBOL_TABLE = SYMBOL_TABLE->traverseOUT();

    codeGenResult res;
    res.type = res_type::VOID;
    return res;
}

codeGenResult AST_conditional::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Conditional not implemented yet");
    return res;
}

codeGenResult AST_loop::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Loop not implemented yet");
    return res;
}

codeGenResult AST_function::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Function not implemented yet");
    return res;
}

codeGenResult AST_function_call::generate_code(){
    if(this->function_name == "write"){
        return CALL_write(this);
    }else if(this->function_name == "read"){
        return CALL_read(this);
    }

    codeGenResult res;
    throw std::runtime_error("Function call not implemented yet");
    return res;
}

codeGenResult AST_return::generate_code(){
    codeGenResult res;
    throw std::runtime_error("Return not implemented yet");
    return res;
}


#endif // CODEGEN_HPP