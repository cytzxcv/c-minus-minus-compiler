#include "static_analyzer.hpp"
#include "node.hpp"

#include <cstdint>
#include <format>

SymbolProperty::SymbolProperty(SymbolType type_, ValType val_type_, std::uint64_t count_)
    : type(type_), val_type(val_type_), count(count_) { }

StaticAnalyzer::StaticAnalyzer() : has_error(false), error("") {
    symbol_table.emplace_back();

    function_table["read"] = 0;
    function_table["fread"] = 0;
    function_table["write"] = 0;
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
            break;
        }
    }
}

void StaticAnalyzer::make_error(std::string error_str, std::uint64_t line) {
    has_error = true;
    error += std::format("{} at [line {}]\n", error_str, line);
}

void StaticAnalyzer::type_decl(Decl::Ptr &decl) {
    TypeDecl *ptr = dynamic_cast<TypeDecl *>(decl.get());

    if (symbol_table.back().contains(ptr->name)) {
        make_error(std::format("Redeclaration of \"{}\"", ptr->name), ptr->line);

        return;
    }

    symbol_table.back().emplace(ptr->name, SymbolProperty(SymbolType::Type, ptr->type, 0));
}

void StaticAnalyzer::var_decl(Decl::Ptr &decl) {
    VarDecl *ptr = dynamic_cast<VarDecl *>(decl.get());

    for (VarDecl::Var &var : ptr->vars) {
        if (symbol_table.back().contains(var.lvalue->name)) {
            make_error(std::format("Redeclaration of \"{}\"", var.lvalue->name), var.lvalue->line);

            continue;
        }
        else if (var.lvalue->dims.size() > 0 && var.expr != nullptr) {
            make_error(std::format("Initialization is not supported for \"{}\"", var.lvalue->name), var.lvalue->line);

            continue;
        }
        else if (var.expr != nullptr /* && eval_type(var.expr) == str */) {
            // report initialization type error

            // continue;
        }

        symbol_table.back().emplace(var.lvalue->name,
                                    SymbolProperty(SymbolType::Var, ptr->type, var.lvalue->dims.size()));
    }
}