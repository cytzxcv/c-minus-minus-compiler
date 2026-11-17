#include "static_analyzer.hpp"
#include "node.hpp"

#include <cstdint>
#include <format>
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
        switch (decl->decl_type) {
        case DeclType::Type:
            type_decl(decl);
            break;
        case DeclType::Var:
            var_decl(decl);
            break;
        case DeclType::Func:
            func_decl(decl);
            break;
        }
    }

    if (function_table.find("main") == function_table.end()) {
        has_error = true;
        error += "No main function\n";
    }
}

void StaticAnalyzer::type_decl(Decl::Ptr &decl) {
    TypeDecl *ptr = dynamic_cast<TypeDecl *>(decl.get());

    if (symbol_table.back().contains(ptr->name)) {
        make_error(std::format("Redeclaration of \"{}\"", ptr->name), ptr->line);

        return;
    }

    symbol_table.back().emplace(ptr->name, SymbolProperty(SymbolType::Type, val_to_eval(ptr->type), 0));
}

void StaticAnalyzer::var_decl(Decl::Ptr &decl) {
    VarDecl *ptr = dynamic_cast<VarDecl *>(decl.get());

    for (VarDecl::Var &var : ptr->vars) {
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

        symbol_table.back().emplace(var.lvalue->name,
                                    SymbolProperty(SymbolType::Var, val_to_eval(ptr->type), var.lvalue->dims.size()));
    }
}

void StaticAnalyzer::func_decl(Decl::Ptr &decl) {
    FuncDecl *ptr = dynamic_cast<FuncDecl *>(decl.get());

    if (symbol_table.back().contains(ptr->name)) {
        make_error(std::format("Redeclaration of \"{}\"", ptr->name), ptr->line);

        return;
    }

    std::unordered_map<std::string_view, SymbolProperty> new_scope_table;

    for (FuncDecl::Param &param : ptr->params) {
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

    symbol_table.back().emplace(ptr->name,
                                SymbolProperty(SymbolType::Func, val_to_eval(ptr->type), ptr->params.size()));

    function_table[ptr->name] = {};
    for (FuncDecl::Param &param : ptr->params) {
        function_table[ptr->name].emplace_back(val_to_eval(param.type));
    }

    symbol_table.emplace_back(new_scope_table);

    // block_stmt(ptr->body)

    symbol_table.pop_back();
}