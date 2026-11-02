#pragma once

#include "lexer.hpp"
#include "node.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
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
    auto decl() -> Decl::Ptr;

    auto type_decl() -> Decl::Ptr;
    auto var_func_decl() -> Decl::Ptr;
    auto var_decl_default() -> VarDecl::Ptr;
    auto var_decl(ValType, LValue::Ptr) -> VarDecl::Ptr;
    auto func_decl(ValType, std::string_view) -> Decl::Ptr;

    auto get_type(Token) -> std::optional<ValType>;

    // stmt
    auto stmt() -> Stmt::Ptr;

    auto block_stmt() -> BlockStmt::Ptr;
    auto expr_stmt() -> Stmt::Ptr;
    auto if_stmt() -> Stmt::Ptr;
    auto for_stmt() -> Stmt::Ptr;
    auto while_stmt() -> Stmt::Ptr;
    auto return_stmt() -> Stmt::Ptr;

    // expr
    auto expr() -> Expr::Ptr;
    auto assignable() -> LValue::Ptr;

    auto expr_bp(std::uint64_t) -> Expr::Ptr;

    auto try_index(Token &, Expr::Ptr &) -> bool;
    auto try_func(Token &, Expr::Ptr &) -> bool;

    constexpr auto infix_bp(OpType) -> std::optional<std::pair<std::uint64_t, std::uint64_t>>;
    constexpr auto prefix_bp(OpType) -> std::optional<std::uint64_t>;

public:
    bool has_error;
    std::string error;

    auto parse() -> Prog::Ptr;

    Parser(Lexer &);
};