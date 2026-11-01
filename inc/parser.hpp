#pragma once

#include "lexer.hpp"
#include "node.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Parser {
private:
    Lexer &lexer;
    std::vector<std::unordered_map<std::string_view, ValType>> type_table;

    auto make_error(std::string, Token &) -> void;
    auto block_item() -> BlockStmt::Item;

    // decl
    auto type_decl() -> Decl::Ptr;

    // stmt
    auto expr_stmt() -> Stmt::Ptr;
    auto if_stmt() -> Stmt::Ptr;

    // expr
    auto expr_bp(std::uint64_t) -> Expr::Ptr;
    auto assignable() -> Expr::Ptr;

    auto try_index(Token &, Expr::Ptr &) -> bool;
    auto try_func(Token &, Expr::Ptr &) -> bool;

    auto infix_bp(OpType) -> std::optional<std::pair<std::uint64_t, std::uint64_t>>;
    auto prefix_bp(OpType) -> std::optional<std::uint64_t>;

public:
    bool has_error;
    std::string error;

    Parser(Lexer &);

    auto expr() -> Expr::Ptr;
};