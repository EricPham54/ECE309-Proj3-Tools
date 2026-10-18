// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

#include <cstddef>
#include <vector>

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out, std::string* error) {
    using Kind = CalcToken::Kind;

    std::vector<double> values;
    std::vector<Kind> ops;

    std::size_t i = 0;
    while (true) {
        const bool at_end = (i == tokens.size());
        const Kind kind = at_end ? Kind::Number : tokens[i].kind;

        bool apply = false;
      
        if (!ops.empty() && (ops.back() != Kind::LParen)) {
            if (at_end || (kind == Kind::RParen)) {
                apply = true;
            } 
            else if (kind == Kind::Plus || kind == Kind::Minus || kind == Kind::Times || kind == Kind::Divide) {
                const int top_prec = (ops.back() == Kind::Times || ops.back() == Kind::Divide) ? 2 : 1;
                const int new_prec = (kind == Kind::Times || kind == Kind::Divide) ? 2 : 1;
                apply = (top_prec >= new_prec);
            }
        }

        if (apply) {
            if (values.size() < 2) {
                *error = "bad expression";
                return false;
            }
            const double right = values.back(); 
            values.pop_back();

            const double left = values.back(); 
            values.pop_back();

            const Kind op = ops.back();    
            ops.pop_back();

            double result = 0.0;
            if (op == Kind::Plus)       result = left + right;
            else if (op == Kind::Minus) result = left - right;
            else if (op == Kind::Times) result = left * right;
            else {
                if (right == 0.0) {
                    *error = "division by zero";
                    return false;
                }
                result = left / right;
            }
            values.push_back(result);
            
            continue;
        }

        if (at_end) {
            if (!ops.empty()) {
                *error = "bad expression";
                return false;
            }
            break;
        }

        if (kind == Kind::Number) {
            values.push_back(tokens[i].value);
        } 
        else if (kind == Kind::RParen) {
            if (ops.empty()) {
                *error = "bad expression";
                return false;
            }
            ops.pop_back();
        } 
        else {
            ops.push_back(kind);
        }
        ++i;
    }

    if (values.size() != 1) {
        *error = "bad expression";
        return false;
    }
    *out = values.back();
    return true;
}
