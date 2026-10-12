#pragma once

#include "lexer.hpp"
#include "node.hpp"

#include <cstdint>
#include <optional>
#include <string>

class Parser {
private:
    Lexer &lexer;
    std::string error;

    std::optional<Expr> expr();
    std::optional<Expr> expr_bp(uint64_t);

public:
    Parser(Lexer &);
};