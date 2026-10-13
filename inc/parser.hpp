#pragma once

#include "lexer.hpp"
#include "node.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

class Parser {
private:
    Lexer &lexer;

    auto make_error(std::string, Token &) -> void;

    auto expr_bp(uint64_t) -> std::optional<Expr::Ptr>;

    auto infix_bp(OpType) -> std::optional<std::pair<uint64_t, uint64_t>>;
    auto prefix_bp(OpType) -> std::optional<std::uint64_t>;

public:
    bool has_error;
    std::string error;

    Parser(Lexer &);

    auto expr() -> std::optional<Expr::Ptr>;
    auto assignable() -> std::optional<Expr::Ptr>;
};