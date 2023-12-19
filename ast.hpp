#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>


class LineNumber {
public:
    static LineNumber& getInstance() {
        static LineNumber instance;
        return instance;
    }

    int getLine() const {
        return line;
    }

    void incrementLine() {
        line++;
    }

private:
    int line = 0; 

    LineNumber() {}

    LineNumber(LineNumber const&) = delete;
    void operator=(LineNumber const&) = delete;
};

//-----------------------------------------------------------------------------------------------------------------------------
// SECTION : ABSTRACT SYNTAX TREE                                                              
//-----------------------------------------------------------------------------------------------------------------------------

enum class AST_type{
    INTEGER,
    CHAR,
    STRING,
    BOOLEAN,
    FLOAT,
    VARIABLE,
    UNARY,
    BINARY,
    CONDITIONAL,
    LOOP,
    FUNCTION,
    RETURN,
    FUNCTION_CALL,
    BLOCK,
};

// RESULT TYPE
enum class res_type{
    INTEGER, CHAR, STRING, FLOAT, BOOLEAN, VOID,
    VAR_INTEGER, VAR_CHAR, VAR_STRING, VAR_FLOAT, VAR_BOOLEAN, VAR_UNKNOWN,
};

// Print result type
std::ostream& operator<<(std::ostream& os, res_type type) {
    switch (type) {
        case res_type::INTEGER:
            os << "INTEGER";
            break;
        case res_type::CHAR:
            os << "CHAR";
            break;
        case res_type::STRING:
            os << "STRING";
            break;
        case res_type::BOOLEAN:
            os << "BOOLEAN";
            break;
        case res_type::FLOAT:
            os << "FLOAT";
            break;
        case res_type::VOID:
            os << "VOID";
            break;
        case res_type::VAR_INTEGER:
            os << "VAR_INTEGER";
            break;
        case res_type::VAR_CHAR:
            os << "VAR_CHAR";
            break;
        case res_type::VAR_STRING:
            os << "VAR_STRING";
            break;
        case res_type::VAR_BOOLEAN:
            os << "VAR_BOOLEAN";
            break;
        case res_type::VAR_FLOAT:
            os << "VAR_FLOAT";
            break;
        case res_type::VAR_UNKNOWN:
            os << "VAR_UNKNOWN";
            break;
    }
    return os;
}

// RESULT
// - data struct that is returned by code generate
struct codeGenResult{
    std::string registerName;  // The name of the register holding the result
    res_type type;            // The data type of the result
    int trueAd;                 // True address
};

// FUNCTION : indentation printing
// - prints the indentation for DEBUGGING purposes only
void print_indent(int indent){
    for(int i = 0; i < indent; i++){
        std::cout << "      ";
    }
}

// BASE CLASS
class AST_expression{
public:
    AST_type type;
    AST_expression(AST_type type) : type(type) {}
    virtual ~AST_expression() = default;
    virtual void print (int) const = 0;
    virtual codeGenResult generate_code() = 0;
};

//  DERIVED CLASS : Integer
//  - Stores an integer literal
class AST_integer : public AST_expression{
public:
    int value;
    AST_integer(int val) : AST_expression(AST_type::INTEGER) {
        this->value = val;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Integer: " << value;
        std::cout << std::endl;
    }

    codeGenResult generate_code();

    ~AST_integer(){}
};

//  DERIVED CLASS : Boolean
//  - Stores a boolean
class AST_boolean : public AST_expression{
public:
    bool value;
    AST_boolean(bool value) : AST_expression(AST_type::BOOLEAN) {
        this->value = value;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Boolean: " << (value ? "true" : "false");
        std::cout << std::endl;
    }

    codeGenResult generate_code();

    ~AST_boolean(){}
};

//  DERIVED CLASS : Float
//  - Stores a float literal
class AST_float : public AST_expression{
public: 
    float value;
    AST_float(float value) : AST_expression(AST_type::FLOAT){
        this->value = value;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Float: " << value;
        std::cout << std::endl;
    }

    codeGenResult generate_code();

    ~AST_float(){}
};

//  DERIVED CLASS : Char
//  - Stores a char literal
class AST_char : public AST_expression{
public:
    char value;
    AST_char(char value) : AST_expression(AST_type::CHAR){
        this->value = value;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Char: '" << value;
        std::cout << std::endl;
    }

    codeGenResult generate_code();

    ~AST_char(){}
};

// DERIVED CLASS : String
// - Stores a string literal
class AST_string : public AST_expression{
public:
    std::string value;
    AST_string(std::string val) : AST_expression(AST_type::STRING) {
        this->value = val;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "String: \"" << value << "\"";
        std::cout << std::endl;
    }

    codeGenResult generate_code();

    ~AST_string(){}
};

//  DERIVED CLASS : Variable
//  - Stores a variable name
class AST_variable : public AST_expression{
private:
    AST_type variableType;
public:
    std::string name;
    AST_variable(std::string name) : AST_expression(AST_type::VARIABLE) {
        this->name = name;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Variable: " << name;
        std::cout << std::endl;
    }
    
    AST_type getVarType() const {
        return variableType;
    }

    void setVarType(AST_type type) {
        variableType = type;
    }

    codeGenResult generate_code();

    ~AST_variable(){}
};

//  DERIVED CLASS : Unary
//  - Stores an operator and an operand
class AST_unary : public AST_expression{
public:
    std::string op;
    AST_expression *expr;

    AST_unary(): AST_expression(AST_type::UNARY){}

    AST_unary(std::string op, AST_expression* expr): AST_expression(AST_type::UNARY){
        this->op = op;
        this->expr = expr;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Unary Expression: " << std::endl;
        print_indent(indent + 1);
        std::cout << op << std::endl;
        expr->print(indent + 1);
    }
    
    codeGenResult generate_code();

    ~AST_unary(){}
};

//  DERIVED CLASS : Binary
//  - Stores an operator and two operands
class AST_binary : public AST_expression{
public:
    std::string op;
    AST_expression *LHS, *RHS;

    AST_binary(): AST_expression(AST_type::BINARY){}

    AST_binary(std::string op, AST_expression* LHS, AST_expression* RHS): AST_expression(AST_type::BINARY){
        this->LHS = LHS;
        this->RHS = RHS;
        this->op = op;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Binary Expression: " << std::endl;
        LHS->print(indent + 1);
        print_indent(indent + 1);
        std::cout << op << std::endl;
        RHS->print(indent + 1);
    }

    codeGenResult generate_code();

    ~AST_binary(){}
};

//  DERIVED CLASS : Block
//  - Stores a list of expressions
class AST_block : public AST_expression{
public:
    std::list <AST_expression*> children;

    AST_block() : AST_expression(AST_type::BLOCK) {}

    void addChild(AST_expression* expr){
        this->children.push_back(expr);
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Block: {" << std::endl;
        for (auto expr : children) {
            expr->print(indent + 1);
        }
        print_indent(indent);
        std::cout << "}" << std::endl;
    }

    codeGenResult generate_code();

    ~AST_block(){};
};

//  DERIVED CLASS : Conditional
//  - Stores a list of branches
class AST_conditional : public AST_expression{
private:
    struct branch{
        AST_expression *condition;
        AST_block *body;
    };
public:
    std::list <branch> branches;

    AST_conditional() : AST_expression(AST_type::CONDITIONAL) {}

    void addBranch(AST_expression* condition, AST_block* body){
        branch b;
        b.condition = condition;
        b.body = body;
        this->branches.push_back(b);
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Conditional: {" << std::endl;
        for (const auto &branch : branches) {
            print_indent(indent+1);

            std::cout << "Condition: " << std::endl;
            if(branch.condition != nullptr){
                branch.condition->print(indent + 1);
            }else{
                print_indent(indent + 2);
                std::cout << "None" << std::endl;
            }

            print_indent(indent+1);
            std::cout << "Body: " <<std::endl;
            branch.body->print(indent + 2);
        }
        print_indent(indent);
        std::cout << "}" << std::endl;
    }
    
    codeGenResult generate_code();

    ~AST_conditional(){}
};

//  DERIVED CLASS : Loop
//  - Stores condition and a body
class AST_loop : public AST_expression{
public:
    AST_expression *condition;
    AST_block *body;

    AST_loop() : AST_expression(AST_type::LOOP) {}

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Loop: {" << std::endl;
        print_indent(indent);
        std::cout << "Condition: " << std::endl;
        condition->print(indent + 1);
        print_indent(indent);
        std::cout << "Body: " << std::endl;
        body->print(indent + 1);
        print_indent(indent);
        std::cout << "}" << std::endl;
    }

    codeGenResult generate_code();

    ~AST_loop(){}
};

//  DERIVED CLASS : Function
//  - Stores a list of parameters and a body
class AST_function : public AST_expression{
public:
    std::string name;
    std::list <AST_expression*> parameters;
    AST_block *body;

    AST_function(std::string name) : AST_expression(AST_type::FUNCTION) {
        this->name = name;
    }

    void addParameter(AST_expression* param){
        this->parameters.push_back(param);
    }

    void setBody(AST_block* body){
        this->body = body;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Function: " << name << "(" << std::endl;
        for (auto param : parameters) {
            param->print(indent+1);
        }
        print_indent(indent);
        std::cout << ") {" << std::endl;
        body->print(indent + 1);
        print_indent(indent);
        std::cout << "}" << std::endl;
    }

    codeGenResult generate_code();

    ~AST_function(){}
};

// DERIVED CLASS : Function call
// - Stores a list of parameters and function's name
class AST_function_call : public AST_expression{
public:
    std::string function_name;
    std::list <AST_expression*> parameters;
    AST_function_call(std::string name, std::list <AST_expression*> parameters) : AST_expression(AST_type::FUNCTION_CALL){
        this->function_name = name;
        this->parameters = parameters;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Function Call: " << function_name << "(" << std::endl;
        for (auto param : parameters) {
            param->print(indent+1);
        }
        print_indent(indent);
        std::cout << ")" << std::endl;
    }
    
    codeGenResult generate_code();
    
    ~AST_function_call(){}
};

// DERIVED CLASS : Return
// - Stores an expression to be returned
class AST_return : public AST_expression{
public:
    AST_expression *expr;
    AST_return(AST_expression* expr) : AST_expression(AST_type::RETURN){
        this->expr = expr;
    }

    void print(int indent) const {
        print_indent(indent);
        std::cout << "Return: " << std::endl;
        expr->print(indent + 1);
    }

    codeGenResult generate_code();

    ~AST_return(){}
};


// CLASS : Program
// - Stores a list of expressions
class AST_program {
public:
    std::list <AST_expression*> expressions;
    AST_program(){}
    void addExpression (AST_expression* expr, int line){
        this->expressions.push_back(expr);
    }

    void print() const {
        std::cout << "Program: {" << std::endl;
        for (auto expr : expressions) {
            expr->print(1);
        }
        std::cout << "}" << std::endl;
    }

    ~AST_program(){}
};


#endif // AST_HPP