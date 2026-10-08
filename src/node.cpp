#include "node.hpp"

#include <string>
#include <utility>

LValue::LValue(std::string name_) : name(name_), dims{} { }

FuncDecl::FuncDecl(ValType type_, std::string name_) : type(type_), name(name_), parems{}, stmts{} { }

VarDecl::VarDecl(ValType type_) : type(type_), vars{} { }

ExprStmt::ExprStmt(Expr::Ptr expr_) : expr(std::move(expr_)) { }

BlockStmt::BlockStmt() : stmts{} { }

IfStmt::IfStmt(Expr::Ptr cond_, Stmt::Ptr if_branch_, Stmt::Ptr else_branch_)
    : cond(std::move(cond_)), if_branch(std::move(if_branch_)), else_branch(std::move(else_branch_)) { }

ForStmt::ForStmt(ForStmt::InitPtr init_, Expr::Ptr cond_, Expr::Ptr iter_, Stmt::Ptr stmt_)
    : init(std::move(init_)), cond(std::move(cond_)), iter(std::move(iter_)), stmt(std::move(stmt_)) { }

WhileStmt::WhileStmt(Expr::Ptr cond_, Stmt::Ptr stmt_) : cond(std::move(cond_)), stmt(std::move(stmt_)) { }

ConstExpr::ConstExpr(ValType type_, std::string value_) : type(type_), value(value_) { }

BinExpr::BinExpr(OpType op_, Expr::Ptr lhs_, Expr::Ptr rhs_) : op(op_), lhs(std::move(lhs_)), rhs(std::move(rhs_)) { }

UnaryExpr::UnaryExpr(OpType op_, Expr::Ptr rhs_) : op(op_), rhs(std::move(rhs_)) { }