#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream> 
#include <string>
#include <list> 
#include <stack>
#include <queue> 

#include "parser.hpp"
#include "table.hpp"


class SyntaxError {
public:
    SyntaxError(const std::string& message, int line)
        : message_("\t======== ERROR FOUND ========\nSyntax Error on or after line " + std::to_string(line) + ": " + message + "\n"), hasError_(true) {}
    SyntaxError() : hasError_(false) {}
    std::string getMessage() const {
        return message_;
    }
    bool hasError() const {
        return hasError_;
    }
private:
    std::string message_;
    bool hasError_;
};


#endif

/* Other Notes: 
    - line counter still inconsistent
        - doesn't work for blocks/scope

    - not sure how to implement with asm
*/



/* 
List of errors (in order of parser.hpp):
    1. invalid declaration 
        1. expected keyword LET
        2. expected identifier
        3. expected data type
    2. token does not match expected token
        1. expected ;
        2. expected \n
        3. expected = 
        4. invalid parameter
        5. invalid expression
        6. not enough operands for operator
        7. left-hand side of assignment is not assignable
        8. no operand for unary operatory
        9. unknown token type
    3. scope error
        1. block missing open brace
        2. block missing close brace
    4. functions
        1. function missing keyword fn
        2. function missing name
        3. function missing open parenthesis
        4. invalid parameter type
        5. invalid parameter
        6. invalid return type
        7. conditional missing open parenthesis
    5. loops
        1. expected keyword WHILE
        2. condition missing open parenthesis
    6. return
        1. expected keyword RETURN
*/


/* try to pass index and check whole code which index the error is found*/

