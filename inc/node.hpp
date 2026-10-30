#pragma once

#include "token.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

enum class ValType {
    Int, // int
    Flt, // float
};

enum class OpType {
    Add, // +
    Sub, // -
    Mul, // *
    Div, // /
    Lt,  // <
    Gt,  // >
    Leq, // <=
    Geq, // >=
    Eq,  // ==
    Neq, // !=
    Or,  // ||
    And, // &&
    Not, // !
    Mov, // =
    Brk, // [
    Prn, // (
};

auto op_type_convert(TokenType) -> OpType;
constexpr auto op_type_to_string(OpType) -> std::string_view;

class Decl {
public:
    using Ptr = std::unique_ptr<Decl>;

    virtual ~Decl() = default;
};

class Stmt {
public:
    using Ptr = std::unique_ptr<Stmt>;

    virtual ~Stmt() = default;
};

class Expr {
public:
    using Ptr = std::unique_ptr<Expr>;

    virtual ~Expr() = default;
    virtual auto print(std::uint64_t indent = 0) const -> void = 0;
};

class LValue : public Expr {
public:
    using Ptr = std::unique_ptr<LValue>;

    std::string_view name;
    std::vector<Expr::Ptr> dims;

    LValue(std::string_view);

    auto print(std::uint64_t) const -> void override;
};

class VarDecl : public Decl {
public:
    using Var = std::pair<LValue::Ptr, Expr::Ptr>;

    ValType type;
    std::vector<Var> vars;

    VarDecl(ValType);
};

class FuncDecl : public Decl {
public:
    using Item = std::variant<std::unique_ptr<VarDecl>, Stmt::Ptr>;
    using Param = std::pair<ValType, LValue::Ptr>;

    ValType type;
    std::string_view name;
    std::vector<Param> params;
    std::vector<Item> body;

    FuncDecl(ValType, std::string_view);
};

class ExprStmt : public Stmt {
public:
    Expr::Ptr expr;

    ExprStmt(Expr::Ptr);
};

class BlockStmt : public Stmt {
public:
    using Item = std::variant<std::unique_ptr<VarDecl>, Stmt::Ptr>;

    std::vector<Item> body;

    BlockStmt();
};

class IfStmt : public Stmt {
public:
    Expr::Ptr cond;
    Stmt::Ptr if_branch, else_branch;

    IfStmt(Expr::Ptr, Stmt::Ptr, Stmt::Ptr);
};

class ForStmt : public Stmt {
public:
    using Init = std::variant<std::monostate, std::unique_ptr<VarDecl>, Expr::Ptr>;

    Init init;
    Expr::Ptr cond, iter;
    Stmt::Ptr stmt;

    ForStmt(Init, Expr::Ptr, Expr::Ptr, Stmt::Ptr);
};

class WhileStmt : public Stmt {
public:
    Expr::Ptr cond;
    Stmt::Ptr stmt;

    WhileStmt(Expr::Ptr, Stmt::Ptr);
};

class ConstExpr : public Expr {
public:
    ValType type;
    std::string_view value;

    ConstExpr(ValType, std::string_view);

    auto print(std::uint64_t) const -> void override;
};

class BinExpr : public Expr {
public:
    OpType op;
    Expr::Ptr lhs, rhs;

    BinExpr(OpType, Expr::Ptr, Expr::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class UnaryExpr : public Expr {
public:
    OpType op;
    Expr::Ptr rhs;

    UnaryExpr(OpType, Expr::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class FuncExpr : public Expr {
public:
    using Ptr = std::unique_ptr<FuncExpr>;

    std::string_view name;
    std::vector<Expr::Ptr> args;

    FuncExpr(std::string_view);

    auto print(std::uint64_t) const -> void override;
};