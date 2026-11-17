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
        case DeclType::Var:
        case DeclType::Func:
        }
    }
}

void StaticAnalyzer::make_error(std::string error_str) {
    has_error = true;
    error += error_str + "\n";
}

void StaticAnalyzer::type_decl(Decl::Ptr &decl) {
    TypeDecl *ptr = dynamic_cast<TypeDecl *>(decl.get());

    if (symbol_table.back().contains(ptr->name)) {
        make_error(std::format("Redeclaration of \"{}\"", ptr->name));

        return;
    }

    symbol_table.back()[ptr->name] = SymbolProperty(SymbolType::Type, ptr->type, 0);
}

void StaticAnalyzer::var_decl(Decl::Ptr &decl) {
    VarDecl *ptr = dynamic_cast<VarDecl *>(decl.get());

    for (VarDecl::Var &var : ptr->vars) {
        if (symbol_table.back().contains(var.lvalue->name)) {
            make_error(std::format("Redeclaration of \"{}\"", var.lvalue->name));

            continue;
        }
        else if (var.lvalue->dims.size() > 0 && var.expr != nullptr) {
            make_error(std::format("Initialization not supported for \"{}\"", var.lvalue->name));

            continue;
        }
        else if (var.expr != nullptr /* && eval_type(var.expr) == str */) {
            // report initialization type error

            // continue;
        }

        symbol_table.back()[var.lvalue->name] = SymbolProperty(SymbolType::Var, ptr->type, var.lvalue->dims.size());
    }
}