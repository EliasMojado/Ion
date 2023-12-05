#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>

#include "ast.hpp"
#include "table.hpp"

//-----------------------------------------------------------------------------------------------------------------------------
// SECTION : LEXICAL ANALYSIS
//-----------------------------------------------------------------------------------------------------------------------------

// ENUMERATION : Token
// - starts at 256 to avoid conflicts with ASCII
enum class Token{
    END_OF_FILE = 256,
    NEW_LINE, UNDEFINED,
    // LITERALS
    INT_literal, FLOAT_literal, BOOL_literal, CHAR_literal, STRING_literal,
    // DATA TYPES
    INT, FLOAT, BOOL, CHAR, STRING, VOID,
    // KEYWORDS
    LET, IF, ELSE, WHILE, RETURN, FUNCTION, CALL,
    // PRIMARY
    IDENTIFIER, SINGLE_OPERATOR, UNARY_OPERATOR, DOUBLE_OPERATOR, SINGLE_COMPARATOR, DOUBLE_COMPARATOR,
    // PUNCTUATION
    CHAR_QUOTE, STRING_QUOTE, COMMA, SEMICOLON, COLON, OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE, OPEN_BRACKET, CLOSE_BRACKET,
};

struct TokenData{
    Token token;
    std::string lexeme = "";
};

// FUNCTION : TOKEN PRINTING
// - prints the token for DEBUGGING purposes only
std::ostream& operator<<(std::ostream& os, Token token) {
    switch (token) {
        case Token::END_OF_FILE: os << "END_OF_FILE"; break;
        case Token::NEW_LINE: os << "NEW_LINE"; break;
        case Token::UNDEFINED: os << "UNDEFINED"; break;
        // LITERALS
        case Token::INT_literal: os << "INT_literal"; break;
        case Token::FLOAT_literal: os << "FLOAT_literal"; break;
        case Token::BOOL_literal: os << "BOOL_literal"; break;
        case Token::CHAR_literal: os << "CHAR_literal"; break;
        case Token::STRING_literal: os << "STRING_literal"; break;
        // DATA TYPES
        case Token::INT: os << "INT"; break;
        case Token::FLOAT: os << "FLOAT"; break;
        case Token::BOOL: os << "BOOL"; break;
        case Token::CHAR: os << "CHAR"; break;
        case Token::STRING: os << "STRING"; break;
        // KEYWORDS
        case Token::LET: os << "LET"; break;
        case Token::IF: os << "IF"; break;
        case Token::ELSE: os << "ELSE"; break;
        case Token::WHILE: os << "WHILE"; break;
        case Token::RETURN: os << "RETURN"; break;
        case Token::FUNCTION: os << "FUNCTION"; break;
        // PRIMARY
        case Token::IDENTIFIER: os << "IDENTIFIER"; break;
        case Token::SINGLE_OPERATOR: os << "SINGLE_OPERATOR"; break;
        case Token::UNARY_OPERATOR: os << "UNARY_OPERATOR"; break;
        case Token::DOUBLE_OPERATOR: os << "DOUBLE_OPERATOR"; break;
        case Token::SINGLE_COMPARATOR: os << "SINGLE_COMPARATOR"; break;
        case Token::DOUBLE_COMPARATOR: os << "DOUBLE_COMPARATOR"; break;
        case Token::CALL: os << "FUNCTION_CALL"; break;
        // PUNCTUATION
        case Token::CHAR_QUOTE: os << "CHAR_QUOTE"; break;
        case Token::STRING_QUOTE: os << "STRING_QUOTE"; break;
        case Token::COMMA: os << "COMMA"; break;
        case Token::SEMICOLON: os << "SEMICOLON"; break;
        case Token::COLON: os << "COLON"; break;
        case Token::OPEN_PAREN: os << "OPEN_PAREN"; break;
        case Token::CLOSE_PAREN: os << "CLOSE_PAREN"; break;
        case Token::OPEN_BRACE: os << "OPEN_BRACE"; break;
        case Token::CLOSE_BRACE: os << "CLOSE_BRACE"; break;
        case Token::OPEN_BRACKET: os << "OPEN_BRACKET"; break;
        case Token::CLOSE_BRACKET: os << "CLOSE_BRACKET"; break;
        default: os << "UNKNOWN TOKEN";
    }
    return os;
}

// FUNCTION :  GET TOKEN 
// - returns the next token in the stream
TokenData get_token(std::string code, int& index){
    TokenData td;
    td.token = Token::UNDEFINED; // default token

    // check for end of file
    if(index >= code.size()){
        td.token = Token::END_OF_FILE;
        return td;
    }
    
    // check for new line
    if(code[index] == '\n'){
        td.token = Token::NEW_LINE;
        index++;
        return td;
    }

    // skip leading whitespace
    while(code[index] == ' ' || code[index] == '\t'){
        index++;
    }

    if(code[index] == '#'){
        while(code[index] != '\n' && code[index] != ';'){
            index++;
        }
        index--;
    }

    // alphabet found
    if(isalpha(code[index])){
        while(isalnum(code[index]) || code[index] == '_'){
            td.lexeme += code[index];
            index++;
        }

        if(td.lexeme == "int"){
            td.token = Token::INT;
        }
        else if(td.lexeme == "float"){
            td.token = Token::FLOAT;
        }
        else if(td.lexeme == "bool"){
            td.token = Token::BOOL;
        }
        else if(td.lexeme == "char"){
            td.token = Token::CHAR;
        }
        else if(td.lexeme == "string"){
            td.token = Token::STRING;
        }
        else if(td.lexeme == "let"){
            td.token = Token::LET;
        }
        else if(td.lexeme == "if"){
            td.token = Token::IF;
        }
        else if(td.lexeme == "else"){
            td.token = Token::ELSE;
        }
        else if(td.lexeme == "while"){
            td.token = Token::WHILE;
        }
        else if(td.lexeme == "return"){
            td.token = Token::RETURN;
        }
        else if(td.lexeme == "fn"){
            td.token = Token::FUNCTION;
        }
        else if(td.lexeme == "void"){
            td.token = Token::VOID;
        }
        else if(td.lexeme == "TRUE" || td.lexeme == "FALSE"){
            td.token = Token::BOOL_literal;
        }
        else{
            td.token = Token::IDENTIFIER;
        }

        int copy_index = index;
        TokenData temp = get_token(code, copy_index);

        if(temp.token == Token::OPEN_PAREN && td.token == Token::IDENTIFIER){
            td.token = Token::CALL;
        }

        return td;
    }else if (isdigit(code[index])){
        td.token = Token::INT_literal;
        while(isdigit(code[index])){
            td.lexeme += code[index];
            index++;
        }
        
        if(code[index] == '.'){
            td.lexeme += code[index];
            index++;
            while(isdigit(code[index])){
                td.lexeme += code[index];
                index++;
            }
            td.token = Token::FLOAT_literal;
        }
        return td;
    }

    // check for double operator first before single operator
    td.lexeme += code[index];
    switch(code[index]){
        case '+':
            td.token = Token::SINGLE_OPERATOR;
            break;
        case '-':
            td.token = Token::SINGLE_OPERATOR;
            break;
        case '*':
            td.token = Token::SINGLE_OPERATOR;
            break;
        case '/':
            td.token = Token::SINGLE_OPERATOR;
            break;
        case '%':
            td.token = Token::SINGLE_OPERATOR;
            break;
        case '=':
            if(code[index+1] == '='){
                td.token = Token::DOUBLE_COMPARATOR;
                td.lexeme += code[index+1];
                index++;
            }else
                td.token = Token::SINGLE_OPERATOR;
            break;
        case '!':
            if(code[index+1] == '='){
                td.token = Token::DOUBLE_COMPARATOR;
                td.lexeme += code[index+1];
                index++;
            }else
                td.token = Token::SINGLE_OPERATOR;
            break;
        case '<':
            if(code[index+1] == '='){
                td.token = Token::DOUBLE_COMPARATOR;
                td.lexeme += code[index+1];
                index++;
            }else
                td.token = Token::SINGLE_COMPARATOR;
            break;
        case '>':
            if(code[index+1] == '='){
                td.token = Token::DOUBLE_COMPARATOR;
                td.lexeme += code[index+1];
                index++;
            }else
                td.token = Token::SINGLE_COMPARATOR;
            break;
        case '&':
            if(code[index+1] == '&'){
                td.token = Token::DOUBLE_OPERATOR;
                td.lexeme += code[index+1];
                index++;
            }else{
                td.token = Token::SINGLE_OPERATOR;
            }
            break;
        case '|':
            if(code[index+1] == '|'){
                td.token = Token::DOUBLE_OPERATOR;
                td.lexeme += code[index+1];
                index++;
            }else{
                td.token = Token::SINGLE_OPERATOR;
            }
            break;
        case '(':
            td.token = Token::OPEN_PAREN;
            break;
        case ')':
            td.token = Token::CLOSE_PAREN;
            break;
        case '{':
            td.token = Token::OPEN_BRACE;
            break;
        case '}':
            td.token = Token::CLOSE_BRACE;
            break;
        case '[':
            td.token = Token::OPEN_BRACKET;
            break;
        case ']':
            td.token = Token::CLOSE_BRACKET;
            break;
        case ',':
            td.token = Token::COMMA;
            break;
        case ';':
            td.token = Token::SEMICOLON;
            break;
        case ':':
            td.token = Token::COLON;
            break;
        case '\'':
            index++;
            td.lexeme = "";
            while(code[index] != '\''){
                td.lexeme += code[index];
                index++;
            }
            td.token = Token::CHAR_literal;
            break;
        case '\"':
            index++;
            td.lexeme = "";
            while(code[index] != '\"'){
                td.lexeme += code[index];
                index++;
            }
            td.token = Token::STRING_literal;
            break;
    }
    index++;

    return td;
}   

//-----------------------------------------------------------------------------------------------------------------------------
// SECTION : SYNTACTIC ANALYSIS
//-----------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------
// SUBSECTION : HELPER FUNCTIONS

// Function : Determine Precedence
int precedence(const TokenData& t){
    // Define the precedence of different operators
    // Higher return value means higher precedence
    if(t.token == Token::UNARY_OPERATOR) return 4;
    if(t.lexeme == "*" || t.lexeme == "/" || t.lexeme == "%") return 3;
    if(t.lexeme == "+" || t.lexeme == "-") return 2;
    if(t.lexeme == "=") return 0;

    return 1;
}

bool is_assignable(AST_expression* expr) {
    // An expression is assignable if it's a variable
    return expr->type == AST_type::VARIABLE;
}

// END OF HELPER FUNCTIONS
//-----------------------------------------------------------------------------------------------------------------------------

AST_program* parse_program(std::string&);
AST_expression* parse_declaration(std::string&, int&);
AST_expression* parse_expression(std::string&, int&, bool);
AST_expression* build_expression(std::queue<TokenData>& operand_queue);
AST_expression* parse_block(std::string&, int&);
AST_expression* parse_function(std::string&, int&);
AST_expression* parse_conditional(std::string&, int&);
AST_expression* parse_loop(std::string&, int&);
AST_expression* parse_return(std::string&, int&);

//  PARSE : Program
//  - this parses the entire program
AST_program* parse_program(std::string &code){    
    AST_program* program = new AST_program();

    int index = 0;
    while(index < code.size()){
        int copy_index = index;
        TokenData td = get_token(code, copy_index);
        if(td.token == Token::NEW_LINE || td.token == Token::SEMICOLON){
            index++;
            continue;
        }

        if(td.token == Token::LET){ // Declaration found
            program->addExpression(parse_declaration(code, index));
        }else if (td.token == Token::FUNCTION){ // Function found
            program->addExpression(parse_function(code, index));
        }else if (td.token == Token::IF){ // Conditional found
            program->addExpression(parse_conditional(code, index));
        }else if (td.token == Token::WHILE){ // Loop found
            program->addExpression(parse_loop(code, index));  
        }else if (td.token == Token::OPEN_BRACE){ 
            program->addExpression(parse_block(code, index));  // block found
        }else if (td.token == Token::RETURN) { 
            program->addExpression(parse_return(code, index)); // return found
        }else{
            program->addExpression(parse_expression(code, index, false));
        }
    }
    return program;
}

//  PARSE: Declarations
//  - this parses a declaration
AST_expression* parse_declaration(std::string &code, int& index){
    AST_expression *LHS, *RHS;
    TokenData t = get_token(code, index);

    std::string name;
    metadata data;

    if(t.token != Token::LET){
        // Error
        throw std::runtime_error("Expected keyword LET in a declaration");
        return nullptr;
    }
    
    // get the Variable
    t = get_token(code, index);
    if(t.token == Token::IDENTIFIER){
        LHS = new AST_variable(t.lexeme);
        name = t.lexeme;
    }else{
        // Error
        throw std::runtime_error("Expected identifier");
    }

    t = get_token(code, index);
    if(t.token == Token::COLON){ // colon, expect data type
        t = get_token(code, index);

        if(t.token == Token::INT){
            // LHS is an integer
            data.type = data_type::INTEGER;
            data.size = 4;
        }else if(t.token == Token::FLOAT){
            // LHS is a float
            data.type = data_type::FLOAT;
            data.size = 4;
        }else if(t.token == Token::BOOL){
            // LHS is a boolean
            data.type = data_type::BOOLEAN;
            data.size = 1;
        }else if(t.token == Token::CHAR){
            // LHS is a char
            data.type = data_type::CHAR;
            data.size = 1;
        }else if(t.token == Token::STRING){
            // LHS is a string
            data.type = data_type::STRING;
            data.size = 8;
        }else{
            // Error
            throw std::runtime_error("Expected data type");
        }
        
        t = get_token(code, index);
    }else{
        // No data type
        data.type = data_type::UNKNOWN;
    }
    
    // Add the variable to the symbol table
    SYMBOL_TABLE->addSymbol(name, data);

    if(t.token == Token::SEMICOLON || t.token == Token::NEW_LINE){
        // Variable 
        return LHS;
    }else if(t.token == Token::SINGLE_OPERATOR && t.lexeme == "="){
        // RHS is a binary expression
        RHS = parse_expression(code, index, false);
        AST_expression* binOpDeclartion = new AST_binary("=", LHS, RHS);
        return binOpDeclartion;
    }else{
        // Error
        throw std::runtime_error("Unexpected Token");
    }


    return nullptr;   
}

//  PARSE: Expression
//  - this parses a general expression. This could be:
//  - Binary operation, function call, variables, literals
//  - This is implemented using the Shunting Yard algorithm
AST_expression* parse_expression(std::string &code, int& index, bool condition = false){
    TokenData t = get_token(code, index);
    TokenData lastToken;
    lastToken.lexeme = "";
    lastToken.token = Token::UNDEFINED;
    std::stack <TokenData> operator_stack;
    std::queue <TokenData> operand_queue;
    AST_expression* expr = nullptr;

    while(t.token != Token::NEW_LINE && t.token != Token::SEMICOLON){        
        if(t.token == Token::OPEN_PAREN){
            operator_stack.push(t);
        }else if(t.token == Token::CLOSE_PAREN){
            while(!operator_stack.empty() && operator_stack.top().token != Token::OPEN_PAREN){
                operand_queue.push(operator_stack.top());
                operator_stack.pop();
            }

            if(operator_stack.empty() && condition){
                break;
            }
            operator_stack.pop();
        }else if(
               t.token == Token::INT_literal
            || t.token == Token::FLOAT_literal
            || t.token == Token::BOOL_literal
            || t.token == Token::CHAR_literal
            || t.token == Token::STRING_literal
            || t.token == Token::IDENTIFIER
        ){
            operand_queue.push(t);
        }else if(t.token == Token::SINGLE_OPERATOR){
            if ((t.token == Token::SINGLE_OPERATOR) && (t.lexeme == "+" || t.lexeme == "-" || t.lexeme == "!")) {
                bool isUnary = (lastToken.token == Token::UNDEFINED ||
                   lastToken.token == Token::OPEN_PAREN ||
                   lastToken.token == Token::SINGLE_OPERATOR ||
                   lastToken.token == Token::DOUBLE_OPERATOR ||
                   lastToken.token == Token::SINGLE_COMPARATOR ||
                   lastToken.token == Token::DOUBLE_COMPARATOR);
                if (isUnary) {
                    t.token = Token::UNARY_OPERATOR;
                }
            }

            while(!operator_stack.empty() 
                && operator_stack.top().token != Token::OPEN_PAREN 
                && precedence(operator_stack.top()) >= precedence(t)
            ){
                operand_queue.push(operator_stack.top());
                operator_stack.pop();
            }
            operator_stack.push(t);
        }else if (t.token == Token::SINGLE_COMPARATOR || t.token == Token::DOUBLE_COMPARATOR || t.token == Token::DOUBLE_OPERATOR){
            while(!operator_stack.empty() 
                && operator_stack.top().token != Token::OPEN_PAREN &&
                precedence(operator_stack.top()) >= precedence(t))
            {
                operand_queue.push(operator_stack.top());
                operator_stack.pop();
            }
            operator_stack.push(t);
        }else if (t.token == Token::CALL){
            operand_queue.push(t);
            TokenData temp = get_token(code, index);

            if(temp.token == Token::OPEN_PAREN){
                operand_queue.push(temp);
            }
            
            temp = get_token(code, index);
            while(temp.token != Token::CLOSE_PAREN){
                if(temp.token == Token::COMMA 
                || temp.token == Token::IDENTIFIER
                || temp.token == Token::INT_literal
                || temp.token == Token::FLOAT_literal
                || temp.token == Token::BOOL_literal
                || temp.token == Token::CHAR_literal
                || temp.token == Token::STRING_literal
                ){
                    operand_queue.push(temp);
                }else{
                    // Error
                    throw std::runtime_error("Invalid parameter");
                }
                temp = get_token(code, index);
            }
            operand_queue.push(temp);
        }

        lastToken = t;
        t = get_token(code, index);
    }

    while(!operator_stack.empty()){
        operand_queue.push(operator_stack.top());
        operator_stack.pop();
    }

    expr = build_expression(operand_queue);

    if(expr == nullptr){
        // Error
        throw std::runtime_error("Invalid expression");
    }

    return expr;
}

// BUILD EXPRESSION
// - this builds the expression from the queue of tokens
// - this is used by the Shunting Yard algorithm
AST_expression* build_expression(std::queue<TokenData>& operand_queue) {
    std::stack<AST_expression*> ast_stack;

    while (!operand_queue.empty()) {
        TokenData t = operand_queue.front();
        // std::cout << t.token << " " << t.lexeme << std::endl;
        operand_queue.pop();
        AST_expression* node;

        // If the token is an operator, pop operands from the stack
        if (t.token == Token::SINGLE_OPERATOR ||
            t.token == Token::DOUBLE_OPERATOR ||
            t.token == Token::SINGLE_COMPARATOR ||
            t.token == Token::DOUBLE_COMPARATOR
        ) {
            if(ast_stack.size() < 2){
                // Error
                throw std::runtime_error("Not enough operands for operator");
            }
            AST_expression* right = ast_stack.top(); ast_stack.pop();
            AST_expression* left = ast_stack.top(); ast_stack.pop();

            // Check if the operator is an assignment operator and if the LHS is assignable
            if (t.lexeme == "=") {
                if (!is_assignable(left)) {
                    // Error
                    throw std::runtime_error("Left-hand side of assignment is not assignable");
                }
            }

            // Set the children of the operator node
            node = new AST_binary(t.lexeme, left, right);
        }else if (t.token == Token::UNARY_OPERATOR) { 
            if (ast_stack.empty()) {
                // Error
                throw std::runtime_error("No operand for unary operator");
            }

            AST_expression* operand = ast_stack.top(); ast_stack.pop();
            node = new AST_unary(t.lexeme, operand);
        }else {
            // Handling for operand tokens
            std::list<AST_expression*> parameters;
            TokenData temp;

            switch (t.token) {
                case Token::INT_literal:
                    node = new AST_integer(std::stoi(t.lexeme));
                    break;
                case Token::FLOAT_literal:
                    node = new AST_float(std::stof(t.lexeme));
                    break;
                case Token::BOOL_literal:
                    node = new AST_boolean(t.lexeme == "TRUE" ? true : false);
                    break;
                case Token::CHAR_literal:
                    node = new AST_char(t.lexeme[0]);
                    break;
                case Token::STRING_literal:
                    node = new AST_string(t.lexeme);
                    break;
                case Token::IDENTIFIER:
                    node = new AST_variable(t.lexeme);
                    break;
                case Token::CALL:
                    temp = t;
                    // Consume the open paren and add all parameters to the function call
                    t = operand_queue.front();
                    operand_queue.pop();
                    if(t.token != Token::OPEN_PAREN){
                        throw std::runtime_error("Function call missing open paren");
                    }
                    
                    t = operand_queue.front();
                    operand_queue.pop();
                    while(t.token != Token::CLOSE_PAREN){
                        if(t.token == Token::IDENTIFIER){
                            parameters.push_back(new AST_variable(t.lexeme));
                        }else if(t.token == Token::INT_literal){
                            parameters.push_back(new AST_integer(std::stoi(t.lexeme)));
                        }else if(t.token == Token::FLOAT_literal){
                            parameters.push_back(new AST_float(std::stof(t.lexeme)));
                        }else if(t.token == Token::BOOL_literal) {
                            parameters.push_back(new AST_boolean(t.lexeme == "TRUE" ? true : false));
                        }else if(t.token == Token::CHAR_literal){
                            parameters.push_back(new AST_char(t.lexeme[0]));
                        }else if(t.token == Token::STRING_literal){
                            parameters.push_back(new AST_string(t.lexeme));
                        }else if(t.token == Token::COMMA){
                            // Do nothing
                        }else{
                            // Error
                            throw std::runtime_error("Invalid parameter");
                        }
                        t = operand_queue.front();
                        operand_queue.pop();
                    }

                    node = new AST_function_call(temp.lexeme, parameters);
                    
                    break;
                default:
                    // Error
                    throw std::runtime_error("Unknown token type");
            }
        }

        // Push the node (operand or operator with its operands) onto the stack
        ast_stack.push(node);
    }

    // The last node on the stack is the root of the AST
    return ast_stack.empty() ? nullptr : ast_stack.top();
}

// PARSE: Block
// - this parses a block of code
// - similar to parse_program, but this is used for parsing blocks{...}
// - this is used by parse_conditional,parse_loop and parse_function
AST_expression* parse_block(std::string &code, int& index){
    AST_block* block = new AST_block();
    TokenData t = get_token(code, index);

    if(t.token != Token::OPEN_BRACE){
        // Error
        throw std::runtime_error("Block missing open brace");
    }
    SYMBOL_TABLE = SYMBOL_TABLE->scopeIn();

    int copy_index = index;
    TokenData td = get_token(code, copy_index);

    while(td.token != Token::CLOSE_BRACE){
        if(td.token == Token::NEW_LINE || td.token == Token::SEMICOLON){
            index++;
        }else if(td.token == Token::END_OF_FILE || code.size() <= index){
            // Error
            throw std::runtime_error("Block missing close brace");
        }else if(td.token == Token::LET){ // Declaration found
            block->addChild(parse_declaration(code, index));
        }else if (td.token == Token::IF){ // Conditional found
            block->addChild(parse_conditional(code, index));
        }else if (td.token == Token::WHILE){ // Loop found
            block->addChild(parse_loop(code, index));  
        }else if (td.token == Token::OPEN_BRACE){  // Scope found
            block->addChild(parse_block(code, index));
        }else if (td.token == Token::RETURN){  // Return found;
            block->addChild(parse_return(code, index));
        }else{
            block->addChild(parse_expression(code, index, false));
        }

        copy_index = index;
        td = get_token(code, copy_index);
    }

    SYMBOL_TABLE = SYMBOL_TABLE->scopeOut();

    index = copy_index;
    // index++;

    return block;
}

//  PARSE: Function
//  - this parses a function which starts with the keyword "fn"
//  - this is used by parse_program ONLY (which means that functions cannot be nested)
AST_expression* parse_function(std::string &code, int& index){
    TokenData t = get_token(code, index);
    if(t.token != Token::FUNCTION){
        // Error
        throw std::runtime_error("Function missing keyword fn");
    }

    t = get_token(code, index);

    AST_function* function;
    if(t.token != Token::CALL){
        // Error
        throw std::runtime_error("Function missing name");
    }else{
        function = new AST_function(t.lexeme);
    }
    
    t = get_token(code, index);
    if(t.token != Token::OPEN_PAREN){
        // Error
        throw std::runtime_error("Function missing open paren");
    }

    while(t.token != Token::CLOSE_PAREN){
        t = get_token(code, index);
        if(t.token == Token::INT_literal){
            function->addParameter(new AST_integer(std::stoi(t.lexeme)));
        }else if(t.token == Token::FLOAT_literal){
            function->addParameter(new AST_float(std::stof(t.lexeme)));
        }else if(t.token == Token::BOOL_literal){
            function->addParameter(new AST_boolean(t.lexeme == "TRUE" ? true : false));
        }else if(t.token == Token::CHAR_literal){
            function->addParameter(new AST_char(t.lexeme[0]));
        }else if(t.token == Token::STRING_literal){
            function->addParameter(new AST_string(t.lexeme));
        }else if(t.token == Token::IDENTIFIER){
            function->addParameter(new AST_variable(t.lexeme));
            int copy_index = index;
            TokenData td = get_token(code, copy_index);
            if(td.token == Token::COLON){ // colon, expect data type
                td = get_token(code, copy_index);

                if(td.token == Token::INT){
                    // Parameter is an integer
                }else if(td.token == Token::FLOAT){
                    // Parameter is a float
                }else if(td.token == Token::BOOL){
                    // Parameter is a boolean
                }else if(td.token == Token::CHAR){
                    // Parameter is a char
                }else if(td.token == Token::STRING){
                    // Parameter is a string
                }else{
                    // Error
                    throw std::runtime_error("Invalid parameter type");
                }
                index = copy_index;
            }
        }else if(t.token == Token::COMMA){ 
            // do nothing
        }else if(t.token == Token::CLOSE_PAREN) { 
            break;
        }else{
            // Error
            throw std::runtime_error("Invalid parameter");
        }
    }

    int copy_index = index;
    TokenData td = get_token(code, copy_index);
    if(td.token == Token::COLON){ // expect data type
        td = get_token(code, copy_index);
        if(td.token == Token::INT){
            // Return type is an integer
        }else if(td.token == Token::FLOAT){
            // Return type is a float
        }else if(td.token == Token::BOOL){
            // Return type is a boolean
        }else if(td.token == Token::CHAR){
            // Return type is a char
        }else if(td.token == Token::STRING){
            // Return type is a string
        }else{
            // Error
            throw std::runtime_error("Invalid return type");
        }

        index = copy_index;
    }

    function->setBody(dynamic_cast<AST_block*>(parse_block(code, index)));
    return function;
}

// PARSE: Conditional
// - this parses a conditional which starts with the keyword "if"
AST_expression* parse_conditional(std::string &code, int& index){
    TokenData t;
    AST_conditional* conditional = new AST_conditional();
    bool elseFound = false;

    while(t.token != Token::END_OF_FILE){
        t = get_token(code, index);
        if(t.token == Token::IF){
            t = get_token(code, index);
            if(t.token != Token::OPEN_PAREN){
                // Error
                throw std::runtime_error("Conditional missing open paren");
            }
            AST_expression* condition = parse_expression(code, index, true);
            AST_block* body = dynamic_cast<AST_block*>(parse_block(code, index));
            conditional->addBranch(condition, body);

            if(elseFound){
                elseFound = false;
            }
        }else if(elseFound){
            // last else
            AST_block* body = dynamic_cast<AST_block*>(parse_block(code, index));
            conditional->addBranch(nullptr, body);
            break;
        }
        
        int copy_index = index;
        t = get_token(code, copy_index);
        if(t.token != Token::ELSE){
            break;
        }else{
            elseFound = true;
            index = copy_index;
        }
    }
    
    return conditional;
}

//  PARSE: Loop
//  - this parses a loop which starts with the keyword "while"
AST_expression* parse_loop(std::string& code, int& index){
    TokenData t;
    AST_loop* loop = new AST_loop();
    t = get_token(code, index);
    if(t.token != Token::WHILE){
        // Error
        throw std::runtime_error("Expected keyword WHILE in a loop");
    }

    t = get_token(code, index);
    if(t.token != Token::OPEN_PAREN){
        // Error
        throw std::runtime_error("Condition missing open paren");
    }

    loop->condition  = parse_expression(code, index, true);
    loop->body = dynamic_cast<AST_block*>(parse_block(code, index));

    return loop;
}

//  PARSE: Return
//  - this parses a return statement
AST_expression* parse_return(std::string& code, int& index){
    TokenData t;
    AST_expression* expr;
    t = get_token(code, index);
    if(t.token != Token::RETURN){
        // Error
        throw std::runtime_error("Expected keyword RETURN");
    }

    expr = parse_expression(code, index, false);
    return new AST_return(expr);
}

#endif