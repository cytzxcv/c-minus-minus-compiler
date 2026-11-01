#include "node.hpp"

#include <print>
#include <string>
#include <string_view>
#include <utility>

auto op_type_convert(TokenType tok_type) -> OpType {
    switch (tok_type) {
    case TokenType::Plus:
        return OpType::Add;
        break;
    case TokenType::Minus:
        return OpType::Sub;
        break;
    case TokenType::Multiply:
        return OpType::Mul;
        break;
    case TokenType::Divide:
        return OpType::Div;
        break;
    case TokenType::Lesser:
        return OpType::Lt;
        break;
    case TokenType::Greater:
        return OpType::Gt;
        break;
    case TokenType::LesserEqual:
        return OpType::Leq;
        break;
    case TokenType::GreaterEqual:
        return OpType::Geq;
        break;
    case TokenType::Equal:
        return OpType::Eq;
        break;
    case TokenType::NotEqual:
        return OpType::Neq;
        break;
    case TokenType::LogicalOr:
        return OpType::Or;
        break;
    case TokenType::LogicalAnd:
        return OpType::And;
        break;
    case TokenType::LogicalNot:
        return OpType::Not;
        break;
    case TokenType::Assign:
        return OpType::Mov;
        break;
    case TokenType::LeftBracket:
        return OpType::Brk;
        break;
    case TokenType::LeftParenthesis:
        return OpType::Prn;
        break;
    default:
    }

    // unreachable
    return OpType::Mov;
}

constexpr auto val_type_to_string(ValType val_type) -> std::string_view {
    switch (val_type) {
    case ValType::Flt:
        return "Float";
        break;
    case ValType::Int:
        return "Int";
        break;

    // unreachable?
    case ValType::Str:
        return "String";
        break;
    }
}

constexpr auto op_type_to_string(OpType op_type) -> std::string_view {
    switch (op_type) {
    case OpType::Add:
        return "+";
        break;
    case OpType::Sub:
        return "-";
        break;
    case OpType::Mul:
        return "*";
        break;
    case OpType::Div:
        return "/";
        break;
    case OpType::Lt:
        return "<";
        break;
    case OpType::Gt:
        return ">";
        break;
    case OpType::Leq:
        return "<=";
        break;
    case OpType::Geq:
        return ">=";
        break;
    case OpType::Eq:
        return "==";
        break;
    case OpType::Neq:
        return "!=";
        break;
    case OpType::Or:
        return "||";
        break;
    case OpType::And:
        return "&&";
        break;
    case OpType::Not:
        return "!";
        break;
    case OpType::Mov:
        return "=";
        break;
    case OpType::Brk:
        return "[]";
        break;
    case OpType::Prn:
        return "()";
        break;
    }
}

LValue::LValue(std::string_view name_) : name(name_), dims{} { }

TypeDecl::TypeDecl(ValType type_, std::string_view name_) : type(type_), name(name_) { }

VarDecl::VarDecl(ValType type_) : type(type_), vars{} { }

FuncDecl::FuncDecl(ValType type_, std::string_view name_) : type(type_), name(name_), params{}, body{} { }

ExprStmt::ExprStmt(Expr::Ptr expr_) : expr(std::move(expr_)) { }

BlockStmt::BlockStmt() : items{} { }

IfStmt::IfStmt(Expr::Ptr cond_, Stmt::Ptr if_branch_, Stmt::Ptr else_branch_)
    : cond(std::move(cond_)), if_branch(std::move(if_branch_)), else_branch(std::move(else_branch_)) { }

ForStmt::ForStmt(ForStmt::Init init_, Expr::Ptr cond_, Expr::Ptr iter_, Stmt::Ptr stmt_)
    : init(std::move(init_)), cond(std::move(cond_)), iter(std::move(iter_)), stmt(std::move(stmt_)) { }

WhileStmt::WhileStmt(Expr::Ptr cond_, Stmt::Ptr stmt_) : cond(std::move(cond_)), stmt(std::move(stmt_)) { }

ConstExpr::ConstExpr(ValType type_, std::string_view value_) : type(type_), value(value_) { }

BinExpr::BinExpr(OpType op_, Expr::Ptr lhs_, Expr::Ptr rhs_) : op(op_), lhs(std::move(lhs_)), rhs(std::move(rhs_)) { }

UnaryExpr::UnaryExpr(OpType op_, Expr::Ptr rhs_) : op(op_), rhs(std::move(rhs_)) { }

FuncExpr::FuncExpr(std::string_view name_) : name(name_), args{} { }

auto TypeDecl::print(std::uint64_t indent = 0) const -> void {
    std::println("{}TypeDecl {} {}", std::string(indent, ' '), val_type_to_string(type), name);
}

auto VarDecl::print(std::uint64_t indent = 0) const -> void { }

auto FuncDecl::print(std::uint64_t indent = 0) const -> void { }

auto ExprStmt::print(std::uint64_t indent = 0) const -> void {
    std::println("{}ExprStmt (", std::string(indent, ' '));

    expr->print(indent + 2);

    std::println("{})", std::string(indent, ' '));
}

auto BlockStmt::print(std::uint64_t indent = 0) const -> void {
    std::println("{}BlockStmt (", std::string(indent, ' '));

    if (items.empty()) {
        std::println("{}*empty*\n{})", std::string(indent + 2, ' '), std::string(indent, ' '));
        return;
    }

    for (const BlockStmt::Item &item : items) {
        std::visit([indent](const auto &ptr) { ptr->print(indent + 2); }, item);
    }

    std::println("{})", std::string(indent, ' '));
}

auto IfStmt::print(std::uint64_t indent = 0) const -> void {
    std::println("{}IfStmt (", std::string(indent, ' '));

    cond->print(indent + 2);
    if_branch->print(indent + 2);

    if (!else_branch) {
        std::println("{})", std::string(indent, ' '));
        return;
    }

    else_branch->print(indent + 2);

    std::println("{})", std::string(indent, ' '));
}

auto ForStmt::print(std::uint64_t indent = 0) const -> void { }

auto WhileStmt::print(std::uint64_t indent = 0) const -> void {
    std::println("{}WhileStmt (", std::string(indent, ' '));

    cond->print(indent + 2);
    stmt->print(indent + 2);

    std::println("{})", std::string(indent, ' '));
}

auto LValue::print(std::uint64_t indent = 0) const -> void {
    std::print("{}LValue {}", std::string(indent, ' '), name);

    if (dims.empty()) {
        std::println();
        return;
    }

    std::println(" (");

    for (const auto &dim : dims) {
        if (dim == nullptr) {
            std::println("{}*empty*", std::string(indent + 2, ' '));
            continue;
        }
        dim->print(indent + 2);
    }

    std::println("{})", std::string(indent, ' '));
}

auto ConstExpr::print(std::uint64_t indent = 0) const -> void {
    std::println("{}ConstExpr {}", std::string(indent, ' '), value);
}

auto BinExpr::print(std::uint64_t indent = 0) const -> void {
    std::println("{}BinExpr {} (", std::string(indent, ' '), op_type_to_string(op));

    lhs->print(indent + 2);
    rhs->print(indent + 2);

    std::println("{})", std::string(indent, ' '));
}

auto UnaryExpr::print(std::uint64_t indent = 0) const -> void {
    std::println("{}UnaryExpr {} (", std::string(indent, ' '), op_type_to_string(op));

    rhs->print(indent + 2);

    std::println("{})", std::string(indent, ' '));
}

auto FuncExpr::print(std::uint64_t indent = 0) const -> void {
    std::print("{}FuncExpr {}", std::string(indent, ' '), name);

    if (args.empty()) {
        std::println(" *empty*");
        return;
    }

    std::println(" (");

    for (const auto &arg : args) {
        arg->print(indent + 2);
    }

    std::println("{})", std::string(indent, ' '));
}