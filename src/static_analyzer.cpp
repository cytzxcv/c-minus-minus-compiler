#include "static_analyzer.hpp"
#include "node.hpp"

#include <cstdint>
#include <format>
#include <variant>
#include <vector>

constexpr EvalType val_to_eval(ValType val_type) {
    switch (val_type) {
    case ValType::Flt:
        return EvalType::Float;
        break;
    case ValType::Int:
        return EvalType::Int;
        break;
    case ValType::Str:
        return EvalType::String;
        break;
    case ValType::Void:
        return EvalType::Void;
        break;
    }
}

SymbolProperty::SymbolProperty(SymbolType type_, EvalType eval_type_, std::uint64_t count_)
    : type(type_), eval_type(eval_type_), count(count_) { }

StaticAnalyzer::StaticAnalyzer() : has_error(false), error("") {
    symbol_table.emplace_back();

    function_table["read"] = {};
    function_table["fread"] = {};
    function_table["write"] = {EvalType::Any};
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
            /*
            if (eval_type(dim) != int) {
                // report dim type error

                // break;
            }
            */
        }

        if (has_dim_error) {
            continue;
        }

        if (var.lvalue->dims.size() > 0 && var.expr != nullptr) {
            make_error(std::format("Initialization is not allowed for array declaration \"{}\"", var.lvalue->name),
                       var.lvalue->line);

            continue;
        }

        if (var.expr != nullptr /* && eval_type(var.expr) != ptr->type */) {
            // report initialization type error

            // continue;
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

            if (i != 0 && dim == nullptr) {
                make_error(
                    std::format("Empty dimension is only allowed for the first dimension of array parameter \"{}\"",
                                param.lvalue->name),
                    param.lvalue->line);

                return;
            }

            /*
            if (eval_type(dim) != int) {
                // report dim type error

                // return;
            }
            */
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

    block_stmt(func_node->body.get());

    symbol_table.pop_back();
}

void StaticAnalyzer::analyze_stmt(Stmt::Ptr &stmt) {
    switch (stmt->stmt_type) {
    case StmtType::Block:
        block_stmt(dynamic_cast<BlockStmt *>(stmt.get()));
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

void StaticAnalyzer::expr_stmt(ExprStmt *expr_node) { }

void StaticAnalyzer::if_stmt(IfStmt *if_node) { }

void StaticAnalyzer::for_stmt(ForStmt *for_node) { }

void StaticAnalyzer::while_stmt(WhileStmt *while_node) { }

void StaticAnalyzer::return_stmt(ReturnStmt *return_node) { }