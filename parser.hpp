#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>

#include "ast.hpp"
#include "table.hpp"
#include "lexer.hpp"   
#include "error.hpp"

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
AST_expression* parse_block(std::string&, int&, bool);
AST_expression* parse_function(std::string&, int&);
AST_expression* parse_conditional(std::string&, int&);
AST_expression* parse_loop(std::string&, int&);
AST_expression* parse_return(std::string&, int&);

//  PARSE : Program
//  - this parses the entire program

int line_counter = 0; // for error handling, detects line before the error



AST_program* parse_program(std::string &code){    
    AST_program* program = new AST_program();

    int index = 0;
    
    while(index < code.size()){
        int copy_index = index;
        TokenData td = get_token(code, copy_index);
        if(td.token == Token::NEW_LINE || td.token == Token::SEMICOLON || code[index] == ' ' || code[index] == '\t'){
            
            if(td.token == Token::NEW_LINE){
                // std::cout<<"found new line"<<std::endl;
                line_counter++;
            }

            index++;
            continue;

        }
        if(td.token == Token::LET){ // Declaration found
            line_counter++;
            program->addExpression(parse_declaration(code, index));
        }else if (td.token == Token::FUNCTION){ // Function found
            line_counter++;
            program->addExpression(parse_function(code, index));
        }else if (td.token == Token::IF){ // Conditional found
            line_counter++;
            program->addExpression(parse_conditional(code, index));
        }else if (td.token == Token::WHILE){ // Loop found
            line_counter++;
            program->addExpression(parse_loop(code, index));  
        }else if (td.token == Token::OPEN_BRACE){ 
            line_counter++;
            program->addExpression(parse_block(code, index, false));  // block found
        }else if (td.token == Token::RETURN) { 
            line_counter++;
            program->addExpression(parse_return(code, index)); // return found
        }else{
            line_counter++;
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

try{
    if(t.token != Token::LET){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Expected keyword LET in a declaration", line_counter);
        return nullptr;
    }
    
    // get the Variable
    t = get_token(code, index);
    if(t.token == Token::IDENTIFIER){
        LHS = new AST_variable(t.lexeme);
        name = t.lexeme;
    }else{
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Expected identifier", line_counter);
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
            throw Error(ErrorType::SYNTAX_ERROR, "Expected data type", line_counter);
        }
        
        t = get_token(code, index);
    }else{
        // No data type
        data.type = data_type::UNKNOWN;
        data.size = 8;
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
        throw Error(ErrorType::SYNTAX_ERROR, "Unexpected Token", line_counter);
    }

} catch (Error& e) {
    std::cout << e.getMessage() << std::endl;
    exit(1);
    }
    return nullptr;   
}

//  PARSE: Expression
//  - this parses a general expression. This could be:
//  - Binary operation, function call, variables, literals
//  - This is implemented using the Shunting Yard algorithm
AST_expression* parse_expression(std::string &code, int& index, bool condition = false){
try{
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
                    throw Error(ErrorType::RUNTIME_ERROR, "Invalid parameter", line_counter);
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

    //debug
    // while(!operand_queue.empty()){
    //     std::cout << operand_queue.front().lexeme << " " << operand_queue.front().token << std::endl;
    //     operand_queue.pop();
    // }

    expr = build_expression(operand_queue);

    if(expr == nullptr){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Invalid expression", line_counter);
    }

    return expr;

} catch (Error& e) {
    std::cout << e.getMessage() << std::endl;
    exit(1);
}
}

// BUILD EXPRESSION
// - this builds the expression from the queue of tokens
// - this is used by the Shunting Yard algorithm
AST_expression*     build_expression(std::queue<TokenData>& operand_queue) {

try{
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
                throw Error(ErrorType::SYNTAX_ERROR, "Not enough operands for operator", line_counter);
            }
            AST_expression* right = ast_stack.top(); ast_stack.pop();
            AST_expression* left = ast_stack.top(); ast_stack.pop();

            // Check if the operator is an assignment operator and if the LHS is assignable
            if (t.lexeme == "=") {
                if (!is_assignable(left)) {
                    // Error
                    throw Error(ErrorType::SYNTAX_ERROR, "Left-hand side of assignment is not assignable", line_counter);
                }
            }

            // Set the children of the operator node
            node = new AST_binary(t.lexeme, left, right);
        }else if (t.token == Token::UNARY_OPERATOR) { 
            if (ast_stack.empty()) {
                // Error
                throw Error(ErrorType::SYNTAX_ERROR, "No operand for unary operator", line_counter);
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
                    stringLiterals[t.lexeme] = "str_" + std::to_string(stringLiteralCounter++);
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
                        throw Error(ErrorType::SYNTAX_ERROR, "Function call missing open paren", line_counter);
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
                            stringLiterals[t.lexeme] = "str_" + std::to_string(stringLiteralCounter++);
                        }else if(t.token == Token::COMMA){
                            // Do nothing
                        }else{
                            // Error
                            throw Error(ErrorType::SYNTAX_ERROR, "Invalid parameter", line_counter);
                        }
                        t = operand_queue.front();
                        operand_queue.pop();
                    }

                    node = new AST_function_call(temp.lexeme, parameters);
                    
                    break;
                default:
                    // Error
                    throw Error(ErrorType::SYNTAX_ERROR, "Unknown token type", line_counter);
            }
        }

        // Push the node (operand or operator with its operands) onto the stack
        ast_stack.push(node);
    }

    // The last node on the stack is the root of the AST
    return ast_stack.empty() ? nullptr : ast_stack.top();
} catch (Error& e) {
    std::cout << e.getMessage() << std::endl;
    exit(1);
}
}

// PARSE: Block
// - this parses a block of code
// - similar to parse_program, but this is used for parsing blocks{...}
// - this is used by parse_conditional,parse_loop and parse_function
AST_expression* parse_block(std::string &code, int& index, bool is_function = false){
try{
    AST_block* block = new AST_block();
    TokenData t = get_token(code, index);

    if(t.token != Token::OPEN_BRACE){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Block missing open brace", line_counter);
    }

    if(!is_function){
        SYMBOL_TABLE = SYMBOL_TABLE->scopeIn();
    }

    int copy_index = index;
    TokenData td = get_token(code, copy_index);

    while(td.token != Token::CLOSE_BRACE){
        if(td.token == Token::NEW_LINE || td.token == Token::SEMICOLON || code[index] == ' ' || code[index] == '\t'){
            index += 1;
        }else if(td.token == Token::END_OF_FILE || code.size() <= index){
            // Error
            throw Error(ErrorType::SYNTAX_ERROR, "Block missing close brace", line_counter);
        }else if(td.token == Token::LET){ // Declaration found
            block->addChild(parse_declaration(code, index));
        }else if (td.token == Token::IF){ // Conditional found
            block->addChild(parse_conditional(code, index));
        }else if (td.token == Token::WHILE){ // Loop found
            block->addChild(parse_loop(code, index));  
        }else if (td.token == Token::OPEN_BRACE){  // Scope found
            block->addChild(parse_block(code, index, false));
        }else if (td.token == Token::RETURN){  // Return found;
            block->addChild(parse_return(code, index));
        }else{
            block->addChild(parse_expression(code, index, false));
        }

        copy_index = index;
        td = get_token(code, copy_index);
    }

    if(!is_function){
        SYMBOL_TABLE = SYMBOL_TABLE->scopeOut();
    }

    index = copy_index;
    // index++;

    return block;
} catch (Error& e) {
    std::cout << e.getMessage() << std::endl;
    exit(1);
}
}

//  PARSE: Function
//  - this parses a function which starts with the keyword "fn"
//  - this is used by parse_program ONLY (which means that functions cannot be nested)
AST_expression* parse_function(std::string &code, int& index){
    TokenData t = get_token(code, index);
    metadata function_data;
    function_data.is_function = true;
    std::string function_name;
    if(t.token != Token::FUNCTION){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Function missing keyword fn", line_counter);

    }

    if(t.token == Token::NEW_LINE){
        line_counter++;
    }

    t = get_token(code, index);

    AST_function* function;
    if(t.token != Token::CALL){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Function missing name", line_counter);
    }else{
        function = new AST_function(t.lexeme);
        function_name = t.lexeme;
    }
    
    t = get_token(code, index);
    if(t.token != Token::OPEN_PAREN){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Function missing open parenthesis", line_counter);
    }

    SYMBOL_TABLE = SYMBOL_TABLE->scopeIn();

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

            // Add the parameter to the symbol table
            metadata data;
            data.type = data_type::UNKNOWN;
            data.size = 8;
            std::string name = t.lexeme;

            int copy_index = index;
            TokenData td = get_token(code, copy_index);
            if(td.token == Token::COLON){ // colon, expect data type
                td = get_token(code, copy_index);

                if(td.token == Token::INT){
                    // Parameter is an integer
                    data.type = data_type::INTEGER;
                    data.size = 4;
                }else if(td.token == Token::FLOAT){
                    // Parameter is a float
                    data.type = data_type::FLOAT;
                    data.size = 4;
                }else if(td.token == Token::BOOL){
                    // Parameter is a boolean
                    data.type = data_type::BOOLEAN;
                    data.size = 1;
                }else if(td.token == Token::CHAR){
                    // Parameter is a char
                    data.type = data_type::CHAR;
                    data.size = 1;
                }else if(td.token == Token::STRING){
                    // Parameter is a string
                    data.type = data_type::STRING;
                    data.size = 8;
                }else{
                    // Error
                    throw Error(ErrorType::TYPE_ERROR, "Invalid parameter type", line_counter);
                }
                index = copy_index;
            }

            SYMBOL_TABLE->addSymbol(name, data);
        }else if(t.token == Token::COMMA){ 
            // do nothing
        }else if(t.token == Token::CLOSE_PAREN) { 
            break;
        }else{
            // Error
            throw Error(ErrorType::TYPE_ERROR, "Invalid parameter", line_counter);
        }
    }

    int copy_index = index;
    TokenData td = get_token(code, copy_index);
    if(td.token == Token::COLON){ // expect data type
        td = get_token(code, copy_index);
        if(td.token == Token::INT){
            // Return type is an integer
            function_data.type = data_type::INTEGER;
        }else if(td.token == Token::FLOAT){
            // Return type is a float
            function_data.type = data_type::FLOAT;
        }else if(td.token == Token::BOOL){
            // Return type is a boolean
            function_data.type = data_type::BOOLEAN;
        }else if(td.token == Token::CHAR){
            // Return type is a char
            function_data.type = data_type::CHAR;
        }else if(td.token == Token::STRING){
            // Return type is a string
            function_data.type = data_type::STRING;
        }else{
            // Error
            throw Error(ErrorType::TYPE_ERROR, "Invalid return type", line_counter);
        }

        index = copy_index;
    }

    function->setBody(dynamic_cast<AST_block*>(parse_block(code, index, true)));
    SYMBOL_TABLE = SYMBOL_TABLE->scopeOut();
    SYMBOL_TABLE->addSymbol(function_name, function_data);
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
                throw Error(ErrorType::SYNTAX_ERROR, "Conditional missing open parenthesis", line_counter);
            }
            AST_expression* condition = parse_expression(code, index, true);
            AST_block* body = dynamic_cast<AST_block*>(parse_block(code, index, false));
            conditional->addBranch(condition, body);

            if(elseFound){
                elseFound = false;
            }
        }else if(elseFound){
            // last else
            AST_block* body = dynamic_cast<AST_block*>(parse_block(code, index, false));
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
        throw Error(ErrorType::SYNTAX_ERROR, "Expected keyword WHILE in a loop", line_counter);
    }

    t = get_token(code, index);
    if(t.token != Token::OPEN_PAREN){
        // Error
        throw Error(ErrorType::SYNTAX_ERROR, "Condition missing open parenthesis", line_counter);
    }

    loop->condition  = parse_expression(code, index, true);
    loop->body = dynamic_cast<AST_block*>(parse_block(code, index, false));

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
        throw Error(ErrorType::SYNTAX_ERROR, "Expected keyword RETURN", line_counter);
    }

    expr = parse_expression(code, index, false);
    return new AST_return(expr);
}

#endif