#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <queue>

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

    // skip leading whitespace
    while(code[index] == ' ' || code[index] == '\t'){
        index++;
    }
    
    // check for new line
    if(code[index] == '\n'){
        td.token = Token::NEW_LINE;
        index++;
        return td;
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

#endif // LEXER_HPP