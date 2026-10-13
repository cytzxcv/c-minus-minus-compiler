#pragma once

#include "token.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

enum class ValType {
    Int,
    Flt,
};

enum class OpType {
    Add,
    Sub,
    Mul,
    Div,
    Lt,
    Gt,
    Leq,
    Geq,
    Eq,
    Neq,
    Or,
    And,
    Not,
    Mov,
    Brk,
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
    virtual auto print(std::uint64_t indent = 0) -> void = 0;
};

class LValue : public Expr {
public:
    using Ptr = std::unique_ptr<LValue>;

    std::string_view name;
    std::vector<Expr::Ptr> dims;

    LValue(std::string_view);

    auto print(std::uint64_t) -> void override;
};

class FuncDecl : public Decl {
public:
    ValType type;
    std::string_view name;
    std::vector<LValue::Ptr> parems;
    std::vector<Stmt::Ptr> stmts;

    FuncDecl(ValType, std::string_view);
};

class VarDecl : public Decl {
public:
    ValType type;
    std::vector<std::pair<LValue::Ptr, Expr::Ptr>> vars;

    VarDecl(ValType);
};

class ExprStmt : public Stmt {
public:
    Expr::Ptr expr;

    ExprStmt(Expr::Ptr);
};

class BlockStmt : public Stmt {
public:
    std::vector<Stmt::Ptr> stmts;

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
    using InitPtr = std::variant<std::unique_ptr<VarDecl>, std::unique_ptr<Expr>>;

    InitPtr init;
    Expr::Ptr cond, iter;
    Stmt::Ptr stmt;

    ForStmt(InitPtr, Expr::Ptr, Expr::Ptr, Stmt::Ptr);
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

    auto print(std::uint64_t) -> void override;
};

class BinExpr : public Expr {
public:
    OpType op;
    Expr::Ptr lhs, rhs;

    BinExpr(OpType, Expr::Ptr, Expr::Ptr);

    auto print(std::uint64_t) -> void override;
};

class UnaryExpr : public Expr {
public:
    OpType op;
    Expr::Ptr rhs;

    UnaryExpr(OpType, Expr::Ptr);

    auto print(std::uint64_t) -> void override;
};
