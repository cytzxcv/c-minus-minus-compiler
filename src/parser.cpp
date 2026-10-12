#include "parser.hpp"

Parser::Parser(Lexer &lexer_) : lexer(lexer_) { }

std::optional<Expr> Parser::expr() {
    return expr_bp(0);
}

std::optional<Expr> Parser::expr_bp(uint64_t bp) {
    // todo
    (void)bp;
    return std::nullopt;
}