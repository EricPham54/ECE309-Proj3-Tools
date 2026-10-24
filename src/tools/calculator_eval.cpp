// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

#include <cstddef>
#include <vector>

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    using Kind = CalcToken::Kind;

    std::vector<double> values;                                                                                         // stack of numbers waiting to be used
    std::vector<Kind> ops;                                                                                              // stack of operators and '(' waiting to be applied

    std::size_t i = 0;                                                                                                  // the token (index) we are looking at of tokens parameter
    while (true) {                                                                                                      // keeps repeating until the tokens/expression has reached the end
        const bool at_end = (i == tokens.size());                                                                       // checks if the current token (index) is at the end of tokens parameter
        const Kind kind   = at_end ? Kind::Number : tokens[i].kind;                                                     // makes kind of token variable based on the current index of the token

        bool apply = false;                                                                                             // default apply variable of false (saying whether or not to apply operators to values)

        if (!ops.empty() && (ops.back() != Kind::LParen)) {                                                             // checks if the operator on the top of the ops stack must be applied before the current token is dealt with                         
            if (at_end || (kind == Kind::RParen)) {                                                                     // makes apply true if the token is at the end or the current token kind is a )
                apply = true;
            } 
            else if (kind == Kind::Plus || kind == Kind::Minus || kind == Kind::Times || kind == Kind::Divide) {        // checks if the operator is a +, -, *, or /
                const int top_prec = (ops.back() == Kind::Times || ops.back() == Kind::Divide) ? 2 : 1;                 // checks if the top of ops stack has precedence depending on whether or not it's a * or /
                const int new_prec = (kind == Kind::Times || kind == Kind::Divide) ? 2 : 1;                             // checks if the current token has precedence depdending on whether or not it's a * or /
                apply = (top_prec >= new_prec);                                                                         // makes apply true or false if the top of ops stack has equal to or greater precedence than the current token
            }
        }

        if (apply) {                                                                                                    // checks if operators in ops stack has to be applied
            if (values.size() < 2) {                                                                                    // returns false if values stack is less than 2
                *error = "bad expression";
                return false;
            }
            const double right = values.back();                                                                         // gets the value that is right of the operator in the token/expression and pops it from the values stack
            values.pop_back();

            const double left = values.back();                                                                          // gets the value that is left of the operator in the the token/expression and pops it from the values stack
            values.pop_back();

            const Kind op = ops.back();                                                                                 // gets the ops that is being applied from the ops stack and pops it from the values stack
            ops.pop_back();

            double result = 0.0;                                                                                        // creates result variable holder
            if (op == Kind::Plus)       result = left + right;                                                          // if-else statements that will do the operation depending on the kind of op
            else if (op == Kind::Minus) result = left - right;
            else if (op == Kind::Times) result = left * right;
            else {
                if (right == 0.0) {                                                                                     // if kind of op is division and the right value is 0, then give error and return false
                    *error = "division by zero";
                    return false;
                }
                result = left / right;
            }
            values.push_back(result);                                                                                   // push the result variable onto the values stack
            continue;                                                                                                   // restart the while(true) loop to see if there are any more operators that need to be applied
        }

        if (at_end) {                                                                                                   // checks if the current token index is at the end of the token/expression
            if (!ops.empty()) {                                                                                         // if the ops stack isn't empty, then give error (due to there being an unterminated `(` ) and return false
                *error = "unbalanced parentheses";
                return false;
            }
            break;                                                                                                      // break out of the while loop
        }

        if (kind == Kind::Number) {                                                                                     // if the current token's kind is a value, then push the value onto the values stack
            values.push_back(tokens[i].value);
        } 
        else if (kind == Kind::RParen) {                                                                                // if the current kind is `)`, then return error and return false if the ops stack is because there should've been a `(` that triggers the first if statement of the while loop
            if (ops.empty()) {
                *error = "unbalanced parentheses";
                return false;
            }
            ops.pop_back();                                                                                             // pop the `(` off the stack because of the `)`
        } else {
            ops.push_back(kind);                                                                                        // push operator onto the ops stack
        }
        ++i;                                                                                                            // increments the token (index) we will look at
    }

    if (values.size() != 1) {                                                                                           // if at the end of the tokens, give error and return false if the values stack has 0 or more than 1 value
        *error = "bad expression";
        return false;
    }
    *out = values.back();                                                                                               // make out the result of the given valid expression
    return true;                                                                                                        // return true if no error
}
