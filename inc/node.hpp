#pragma once

#include "token.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

enum class ValType {
    Void, // void
    Int,  // int
    Flt,  // float
    Str,  // string
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

constexpr auto val_type_to_string(ValType) -> std::string_view;
constexpr auto op_type_to_string(OpType) -> std::string_view;

class Decl {
public:
    using Ptr = std::unique_ptr<Decl>;

    virtual ~Decl() = default;
    virtual auto print(std::uint64_t indent = 0) const -> void = 0;
};

class Stmt {
public:
    using Ptr = std::unique_ptr<Stmt>;

    virtual ~Stmt() = default;
    virtual auto print(std::uint64_t indent = 0) const -> void = 0;
};

class Expr {
public:
    using Ptr = std::unique_ptr<Expr>;

    virtual ~Expr() = default;
    virtual auto print(std::uint64_t indent = 0) const -> void = 0;
};

class Prog {
public:
    using Ptr = std::unique_ptr<Prog>;

    std::vector<Decl::Ptr> decls;

    Prog();

    auto print() const -> void;
};

class LValue : public Expr {
public:
    using Ptr = std::unique_ptr<LValue>;

    std::string_view name;
    std::vector<Expr::Ptr> dims;

    LValue(std::string_view);

    auto print(std::uint64_t) const -> void override;
};

class BlockStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<BlockStmt>;
    using Item = std::variant<Decl::Ptr, Stmt::Ptr>;

    std::vector<Item> items;

    BlockStmt();

    auto print(std::uint64_t) const -> void override;
};

class VarDecl : public Decl {
public:
    using Ptr = std::unique_ptr<VarDecl>;
    using Var = std::pair<LValue::Ptr, Expr::Ptr>;

    ValType type;
    std::vector<Var> vars;

    VarDecl(ValType);

    auto print(std::uint64_t) const -> void override;
};

class TypeDecl : public Decl {
public:
    ValType type;
    std::string_view name;

    TypeDecl(ValType, std::string_view);

    auto print(std::uint64_t) const -> void override;
};

class FuncDecl : public Decl {
public:
    using Param = std::pair<ValType, LValue::Ptr>;

    ValType type;
    std::string_view name;
    std::vector<Param> params;
    BlockStmt::Ptr body;

    FuncDecl(ValType, std::string_view);

    auto print(std::uint64_t) const -> void override;
};

class ExprStmt : public Stmt {
public:
    Expr::Ptr expr;

    ExprStmt(Expr::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class IfStmt : public Stmt {
public:
    Expr::Ptr cond;
    Stmt::Ptr if_branch, else_branch;

    IfStmt(Expr::Ptr, Stmt::Ptr, Stmt::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class ForStmt : public Stmt {
public:
    using Init = std::variant<std::monostate, VarDecl::Ptr, Expr::Ptr>;

    Init init;
    Expr::Ptr cond, iter;
    Stmt::Ptr stmt;

    ForStmt(Init, Expr::Ptr, Expr::Ptr, Stmt::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class WhileStmt : public Stmt {
public:
    Expr::Ptr cond;
    Stmt::Ptr stmt;

    WhileStmt(Expr::Ptr, Stmt::Ptr);

    auto print(std::uint64_t) const -> void override;
};

class ReturnStmt : public Stmt {
public:
    Expr::Ptr expr;

    ReturnStmt(Expr::Ptr);

    auto print(std::uint64_t) const -> void override;
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
    std::string_view name;
    std::vector<Expr::Ptr> args;

    FuncExpr(std::string_view);

    auto print(std::uint64_t) const -> void override;
};