#pragma once

#include "node.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class EvalType {
    Int,
    Float,
    String,
    Void,
    Err,
    Any,
};

constexpr EvalType val_to_eval(ValType);
constexpr std::string_view eval_to_str(EvalType);

enum class SymbolType {
    Var,
    Type,
    Func,
};

class SymbolProperty {
public:
    SymbolType type;
    EvalType eval_type;
    std::uint64_t count; // dim for var, arg count for func

    SymbolProperty(SymbolType, EvalType, std::uint64_t);
};

class StaticAnalyzer {
private:
    std::vector<std::unordered_map<std::string_view, SymbolProperty>> symbol_table;
    std::unordered_map<std::string_view, std::vector<EvalType>> function_table;
    std::string_view analyzing_function;

    std::optional<SymbolProperty> get_symbol(std::string_view);
    bool type_compatible(EvalType, EvalType);
    EvalType bin_eval_type(EvalType, OpType, EvalType);
    EvalType unary_eval_type(OpType, EvalType);
    void make_error(std::string, std::uint64_t);

    // decl
    void analyze_decl(Decl::Ptr &);

    void type_decl(TypeDecl *);
    void var_decl(VarDecl *);
    void func_decl(FuncDecl *);

    // stmt
    void analyze_stmt(Stmt::Ptr &);

    void block_stmt(BlockStmt *);
    void expr_stmt(ExprStmt *);
    void if_stmt(IfStmt *);
    void for_stmt(ForStmt *);
    void while_stmt(WhileStmt *);
    void return_stmt(ReturnStmt *);

    // expr
    EvalType analyze_expr(Expr::Ptr &);

    EvalType lvalue(LValue *);
    EvalType const_expr(ConstExpr *);
    EvalType bin_expr(BinExpr *);
    EvalType unary_expr(UnaryExpr *);
    EvalType func_expr(FuncExpr *);

public:
    bool has_error;
    std::string error;

    void analyze(Prog::Ptr &);

    StaticAnalyzer();
};