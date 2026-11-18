#include "static_analyzer.hpp"
#include "node.hpp"

#include <cstdint>
#include <format>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

constexpr EvalType val_to_eval(ValType val_type) {
    switch (val_type) {
    case ValType::Flt:
        return EvalType::Float;

    case ValType::Int:
        return EvalType::Int;

    case ValType::Str:
        return EvalType::String;

    case ValType::Void:
        return EvalType::Void;
    }
}

constexpr std::string_view eval_to_str(EvalType eval_type) {
    switch (eval_type) {
    case EvalType::Int:
        return "int";

    case EvalType::Float:
        return "float";

    case EvalType::String:
        return "string";

    case EvalType::Void:
        return "void";

    case EvalType::Err:
        return "err";

    case EvalType::Any:
        return "any";
    }
}

SymbolProperty::SymbolProperty(SymbolType type_, EvalType eval_type_, std::uint64_t count_)
    : type(type_), eval_type(eval_type_), count(count_) { }

StaticAnalyzer::StaticAnalyzer() : has_error(false), error("") {
    symbol_table.emplace_back();

    symbol_table.back().emplace("read", SymbolProperty(SymbolType::Func, EvalType::Int, 0));
    symbol_table.back().emplace("fread", SymbolProperty(SymbolType::Func, EvalType::Float, 0));
    symbol_table.back().emplace("write", SymbolProperty(SymbolType::Func, EvalType::Void, 1));

    function_table["read"] = {};
    function_table["fread"] = {};
    function_table["write"] = {EvalType::Any};
}

std::optional<SymbolProperty> StaticAnalyzer::get_symbol(std::string_view name) {
    for (auto it = symbol_table.rbegin(); it != symbol_table.rend(); it++) {
        if (it->contains(name)) {
            return it->at(name);
        }
    }

    return std::nullopt;
}

bool StaticAnalyzer::type_compatible(EvalType trying_type, EvalType result_type) {
    if (trying_type == EvalType::Void || result_type == EvalType::Void || trying_type == EvalType::Err ||
        result_type == EvalType::Err || trying_type == EvalType::Any) {
        return false;
    }
    else if (result_type == EvalType::Any) {
        return true;
    }
    else if (trying_type == EvalType::Int && result_type == EvalType::Float) {
        return true;
    }

    return trying_type == result_type;
}

EvalType StaticAnalyzer::bin_eval_type(EvalType lhs, OpType op, EvalType rhs) {
    if (lhs == EvalType::String || rhs == EvalType::String || lhs == EvalType::Void || rhs == EvalType::Void ||
        lhs == EvalType::Err || rhs == EvalType::Err || lhs == EvalType::Any || rhs == EvalType::Any) {
        return EvalType::Err;
    }
    else if (op == OpType::Cma) {
        return rhs;
    }
    else if (op == OpType::Mov) {
        return type_compatible(rhs, lhs) ? lhs : EvalType::Err;
    }

    switch (op) {
    case OpType::Add:
    case OpType::Sub:
    case OpType::Mul:
    case OpType::Div:
        return lhs == EvalType::Float || rhs == EvalType::Float ? EvalType::Float : EvalType::Int;

    case OpType::Lt:
    case OpType::Gt:
    case OpType::Leq:
    case OpType::Geq:
    case OpType::Eq:
    case OpType::Neq:
    case OpType::Or:
    case OpType::And:
        return EvalType::Int;

    default:
        return EvalType::Err;
    }
}

EvalType StaticAnalyzer::unary_eval_type(OpType op, EvalType rhs) {
    if (rhs == EvalType::String || rhs == EvalType::Void || rhs == EvalType::Err || rhs == EvalType::Any) {
        return EvalType::Err;
    }

    switch (op) {
    case OpType::Add:
    case OpType::Sub:
        return rhs;

    case OpType::Not:
        return EvalType::Int;

    default:
        return EvalType::Err;
    }
}

void StaticAnalyzer::make_error(std::string error_str, std::uint64_t line) {
    has_error = true;
    error += std::format("{} at [line {}]\n", error_str, line);
}

void StaticAnalyzer::analyze(Prog::Ptr &prog) {
    for (Decl::Ptr &decl : prog->decls) {
        analyze_decl(decl);
    }

    if (function_table.find("main") == function_table.end()) {
        has_error = true;
        error += "No main function\n";
    }
}

// decl
void StaticAnalyzer::analyze_decl(Decl::Ptr &decl) {
    switch (decl->decl_type) {
    case DeclType::Type:
        type_decl(dynamic_cast<TypeDecl *>(decl.get()));
        break;

    case DeclType::Var:
        var_decl(dynamic_cast<VarDecl *>(decl.get()));
        break;

    case DeclType::Func:
        func_decl(dynamic_cast<FuncDecl *>(decl.get()));
        break;
    }
}

void StaticAnalyzer::type_decl(TypeDecl *type_node) {
    if (symbol_table.back().contains(type_node->name)) {
        make_error(std::format("Redeclaration of \"{}\"", type_node->name), type_node->line);

        return;
    }

    symbol_table.back().emplace(type_node->name, SymbolProperty(SymbolType::Type, val_to_eval(type_node->type), 0));
}

void StaticAnalyzer::var_decl(VarDecl *var_node) {
    for (VarDecl::Var &var : var_node->vars) {
        if (symbol_table.back().contains(var.lvalue->name)) {
            make_error(std::format("Redeclaration of \"{}\"", var.lvalue->name), var.lvalue->line);

            continue;
        }

        bool has_dim_error = false;
        for (Expr::Ptr &dim : var.lvalue->dims) {
            if (dim == nullptr) {
                make_error(std::format("Empty dimension is not allowed for array declaration \"{}\"", var.lvalue->name),
                           var.lvalue->line);

                has_dim_error = true;

                break;
            }

            EvalType dim_type = analyze_expr(dim);

            if (dim_type == EvalType::Err) {
                has_dim_error = true;

                break;
            }
            else if (dim_type != EvalType::Int) {
                make_error(std::format("Non-integer dimension is not allowed for array \"{}\"", var.lvalue->name),
                           var.lvalue->line);

                has_dim_error = true;

                break;
            }
        }

        if (has_dim_error) {
            continue;
        }

        if (var.lvalue->dims.size() > 0 && var.expr != nullptr) {
            make_error(std::format("Initialization is not allowed for array declaration \"{}\"", var.lvalue->name),
                       var.lvalue->line);

            continue;
        }

        if (var.expr != nullptr && !type_compatible(analyze_expr(var.expr), val_to_eval(var_node->type))) {
            make_error(std::format("Initialization type incompatible \"{}\"", var.lvalue->name), var.lvalue->line);

            continue;
        }

        symbol_table.back().emplace(
            var.lvalue->name, SymbolProperty(SymbolType::Var, val_to_eval(var_node->type), var.lvalue->dims.size()));
    }
}

void StaticAnalyzer::func_decl(FuncDecl *func_node) {
    if (symbol_table.back().contains(func_node->name)) {
        make_error(std::format("Redeclaration of \"{}\"", func_node->name), func_node->line);

        return;
    }

    std::unordered_map<std::string_view, SymbolProperty> new_scope_table;

    for (FuncDecl::Param &param : func_node->params) {
        if (new_scope_table.contains(param.lvalue->name)) {
            make_error(std::format("Redeclaration of \"{}\"", param.lvalue->name), param.lvalue->line);

            return;
        }

        for (std::uint64_t i = 0; i < param.lvalue->dims.size(); i++) {
            Expr::Ptr &dim = param.lvalue->dims[i];

            if (i == 0 && dim == nullptr) {
                continue;
            }
            else if (i != 0 && dim == nullptr) {
                make_error(
                    std::format("Empty dimension is only allowed for the first dimension of array parameter \"{}\"",
                                param.lvalue->name),
                    param.lvalue->line);

                return;
            }

            EvalType dim_type = analyze_expr(dim);

            if (dim_type == EvalType::Err) {
                return;
            }
            else if (dim_type != EvalType::Int) {
                make_error(std::format("Non-integer dimension is not allowed for array \"{}\"", param.lvalue->name),
                           param.lvalue->line);

                return;
            }
        }

        new_scope_table.emplace(param.lvalue->name,
                                SymbolProperty(SymbolType::Var, val_to_eval(param.type), param.lvalue->dims.size()));
    }

    symbol_table.back().emplace(
        func_node->name, SymbolProperty(SymbolType::Func, val_to_eval(func_node->type), func_node->params.size()));

    function_table[func_node->name] = {};
    for (FuncDecl::Param &param : func_node->params) {
        function_table[func_node->name].emplace_back(val_to_eval(param.type));
    }

    symbol_table.emplace_back(new_scope_table);

    analyzing_function = func_node->name;
    block_stmt(func_node->body.get());

    symbol_table.pop_back();
}

// stmt
void StaticAnalyzer::analyze_stmt(Stmt::Ptr &stmt) {
    switch (stmt->stmt_type) {
    case StmtType::Block:
        symbol_table.emplace_back();
        block_stmt(dynamic_cast<BlockStmt *>(stmt.get()));
        symbol_table.pop_back();
        break;

    case StmtType::Expr:
        expr_stmt(dynamic_cast<ExprStmt *>(stmt.get()));
        break;

    case StmtType::If:
        if_stmt(dynamic_cast<IfStmt *>(stmt.get()));
        break;

    case StmtType::For:
        for_stmt(dynamic_cast<ForStmt *>(stmt.get()));
        break;

    case StmtType::While:
        while_stmt(dynamic_cast<WhileStmt *>(stmt.get()));
        break;

    case StmtType::Return:
        return_stmt(dynamic_cast<ReturnStmt *>(stmt.get()));
        break;
    }
}

void StaticAnalyzer::block_stmt(BlockStmt *block_node) {
    for (BlockStmt::Item &item : block_node->items) {
        if (std::holds_alternative<Stmt::Ptr>(item)) {
            analyze_stmt(std::get<Stmt::Ptr>(item));
        }
        else {
            analyze_decl(std::get<Decl::Ptr>(item));
        }
    }
}

void StaticAnalyzer::expr_stmt(ExprStmt *expr_node) {
    analyze_expr(expr_node->expr);
}

void StaticAnalyzer::if_stmt(IfStmt *if_node) {
    EvalType cond_type = analyze_expr(if_node->cond);

    if (cond_type == EvalType::String || cond_type == EvalType::Void) {
        make_error(std::format("Unavailable condition type {}", eval_to_str(cond_type)), if_node->line);
    }

    analyze_stmt(if_node->if_branch);

    if (if_node->else_branch != nullptr) {
        analyze_stmt(if_node->else_branch);
    }
}

void StaticAnalyzer::for_stmt(ForStmt *for_node) {
    symbol_table.emplace_back();

    if (std::holds_alternative<VarDecl::Ptr>(for_node->init)) {
        var_decl(dynamic_cast<VarDecl *>(std::get<VarDecl::Ptr>(for_node->init).get()));
    }
    else if (std::holds_alternative<Expr::Ptr>(for_node->init)) {
        analyze_expr(std::get<Expr::Ptr>(for_node->init));
    }

    if (for_node->cond != nullptr) {
        EvalType cond_type = analyze_expr(for_node->cond);

        if (cond_type == EvalType::String || cond_type == EvalType::Void) {
            make_error(std::format("Unavailable condition type {}", eval_to_str(cond_type)), for_node->line);
        }
    }

    if (for_node->iter != nullptr) {
        analyze_expr(for_node->iter);
    }

    analyze_stmt(for_node->stmt);

    symbol_table.pop_back();
}

void StaticAnalyzer::while_stmt(WhileStmt *while_node) {
    EvalType cond_type = analyze_expr(while_node->cond);

    if (cond_type == EvalType::String || cond_type == EvalType::Void) {
        make_error(std::format("Unavailable condition type {}", eval_to_str(cond_type)), while_node->line);
    }

    analyze_stmt(while_node->stmt);
}

void StaticAnalyzer::return_stmt(ReturnStmt *return_node) {
    EvalType return_type;

    if (return_node->expr == nullptr) {
        return_type = EvalType::Void;
    }
    else {
        return_type = analyze_expr(return_node->expr);
    }

    if (return_type == EvalType::Err) {
        return;
    }
    else if (!type_compatible(return_type, symbol_table[0].at(analyzing_function).eval_type)) {
        make_error(std::format("Return type mismatch \"{}\"", analyzing_function), return_node->line);
    }
}

// expr
EvalType StaticAnalyzer::analyze_expr(Expr::Ptr &expr) {
    switch (expr->expr_type) {
    case ExprType::LValue:
        return lvalue(dynamic_cast<LValue *>(expr.get()));

    case ExprType::Const:
        return const_expr(dynamic_cast<ConstExpr *>(expr.get()));

    case ExprType::Bin:
        return bin_expr(dynamic_cast<BinExpr *>(expr.get()));

    case ExprType::Unary:
        return unary_expr(dynamic_cast<UnaryExpr *>(expr.get()));

    case ExprType::Func:
        return func_expr(dynamic_cast<FuncExpr *>(expr.get()));
    }
}

EvalType StaticAnalyzer::lvalue(LValue *lvalue_node) {
    std::optional<SymbolProperty> symbol = get_symbol(lvalue_node->name);

    if (!symbol) {
        make_error(std::format("Undeclared symbol \"{}\"", lvalue_node->name), lvalue_node->line);

        return EvalType::Err;
    }
    else if (symbol.value().type != SymbolType::Var) {
        make_error(std::format("Symbol is not a variable \"{}\"", lvalue_node->name), lvalue_node->line);

        return EvalType::Err;
    }
    else if (lvalue_node->dims.size() != symbol.value().count) {
        make_error(std::format("Dimension count mismatch \"{}\"", lvalue_node->name), lvalue_node->line);

        return EvalType::Err;
    }

    for (Expr::Ptr &dim : lvalue_node->dims) {
        EvalType dim_type = analyze_expr(dim);

        if (dim_type == EvalType::Err) {
            return EvalType::Err;
        }
        else if (dim_type != EvalType::Int) {
            make_error(std::format("Non-integer dimension is not allowed for array \"{}\"", lvalue_node->name),
                       lvalue_node->line);

            return EvalType::Err;
        }
    }

    return symbol->eval_type;
}

EvalType StaticAnalyzer::const_expr(ConstExpr *const_node) {
    return val_to_eval(const_node->type);
}

EvalType StaticAnalyzer::bin_expr(BinExpr *bin_node) {
    EvalType lhs = analyze_expr(bin_node->lhs);
    EvalType rhs = analyze_expr(bin_node->rhs);

    if (lhs == EvalType::Err || rhs == EvalType::Err) {
        return EvalType::Err;
    }

    EvalType result = bin_eval_type(lhs, bin_node->op, rhs);

    if (result == EvalType::Err) {
        make_error(std::format("Unsupported operation {} between {} and {}", op_type_to_string(bin_node->op),
                               eval_to_str(lhs), eval_to_str(rhs)),
                   bin_node->line);

        return EvalType::Err;
    }

    return result;
}

EvalType StaticAnalyzer::unary_expr(UnaryExpr *unary_node) {
    EvalType rhs = analyze_expr(unary_node->rhs);

    if (rhs == EvalType::Err) {
        return EvalType::Err;
    }

    EvalType result = unary_eval_type(unary_node->op, rhs);

    if (result == EvalType::Err) {
        make_error(std::format("Unsupported operation {} on {}", op_type_to_string(unary_node->op), eval_to_str(rhs)),
                   unary_node->line);

        return EvalType::Err;
    }

    return result;
}

EvalType StaticAnalyzer::func_expr(FuncExpr *func_node) {
    std::optional<SymbolProperty> symbol = get_symbol(func_node->name);

    if (!symbol) {
        make_error(std::format("Undeclared symbol \"{}\"", func_node->name), func_node->line);

        return EvalType::Err;
    }
    else if (symbol.value().type != SymbolType::Func) {
        make_error(std::format("Symbol is not a function \"{}\"", func_node->name), func_node->line);

        return EvalType::Err;
    }
    else if (func_node->args.size() != symbol.value().count) {
        make_error(std::format("Function argument count mismatch \"{}\"", func_node->name), func_node->line);

        return EvalType::Err;
    }

    for (std::uint64_t i = 0; i < func_node->args.size(); i++) {
        EvalType arg_type = analyze_expr(func_node->args[i]);

        if (arg_type == EvalType::Err) {
            return EvalType::Err;
        }
        else if (!type_compatible(arg_type, function_table[func_node->name][i])) {
            make_error(std::format("Function argument type mismatch \"{}\"", func_node->name), func_node->line);

            return EvalType::Err;
        }
    }

    return symbol->eval_type;
}
