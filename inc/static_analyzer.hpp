#pragma once

#include "node.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class SymbolType {
    Var,
    Type,
    Func,
};

class SymbolProperty {
public:
    SymbolType type;
    ValType val_type;
    std::uint64_t count; // dim for var, arg count for func

    SymbolProperty(SymbolType, ValType, std::uint64_t);
};

class StaticAnalyzer {
private:
    std::vector<std::unordered_map<std::string_view, SymbolProperty>> symbol_table;
    std::unordered_map<std::string_view, std::uint64_t> function_table;

    void make_error(std::string, std::uint64_t);

    void type_decl(Decl::Ptr &);
    void var_decl(Decl::Ptr &);

public:
    bool has_error;
    std::string error;

    void analyze(Prog::Ptr &);

    StaticAnalyzer();
};