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
    default:
    }

    // unreachable
    return OpType::Mov;
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
    }
}

LValue::LValue(std::string_view name_) : name(name_), dims{} { }

FuncDecl::FuncDecl(ValType type_, std::string_view name_) : type(type_), name(name_), parems{}, stmts{} { }

VarDecl::VarDecl(ValType type_) : type(type_), vars{} { }

ExprStmt::ExprStmt(Expr::Ptr expr_) : expr(std::move(expr_)) { }

BlockStmt::BlockStmt() : stmts{} { }

IfStmt::IfStmt(Expr::Ptr cond_, Stmt::Ptr if_branch_, Stmt::Ptr else_branch_)
    : cond(std::move(cond_)), if_branch(std::move(if_branch_)), else_branch(std::move(else_branch_)) { }

ForStmt::ForStmt(ForStmt::InitPtr init_, Expr::Ptr cond_, Expr::Ptr iter_, Stmt::Ptr stmt_)
    : init(std::move(init_)), cond(std::move(cond_)), iter(std::move(iter_)), stmt(std::move(stmt_)) { }

WhileStmt::WhileStmt(Expr::Ptr cond_, Stmt::Ptr stmt_) : cond(std::move(cond_)), stmt(std::move(stmt_)) { }

ConstExpr::ConstExpr(ValType type_, std::string_view value_) : type(type_), value(value_) { }

BinExpr::BinExpr(OpType op_, Expr::Ptr lhs_, Expr::Ptr rhs_) : op(op_), lhs(std::move(lhs_)), rhs(std::move(rhs_)) { }

UnaryExpr::UnaryExpr(OpType op_, Expr::Ptr rhs_) : op(op_), rhs(std::move(rhs_)) { }

auto LValue::print(uint64_t indent = 0) -> void {
    std::println("{}{}", std::string(indent, ' '), name);
}

auto ConstExpr::print(uint64_t indent = 0) -> void {
    std::println("{}{}", std::string(indent, ' '), value);
}

auto BinExpr::print(uint64_t indent = 0) -> void {
    std::println("{}{} (", std::string(indent, ' '), op_type_to_string(op));
    lhs->print(indent + 2);
    rhs->print(indent + 2);
    std::println("{})", std::string(indent, ' '));
}

auto UnaryExpr::print(uint64_t indent = 0) -> void {
    std::println("{}{} (", std::string(indent, ' '), op_type_to_string(op));
    rhs->print(indent + 2);
    std::println("{})", std::string(indent, ' '));
}