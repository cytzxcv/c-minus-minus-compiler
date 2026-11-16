#pragma once

#include "node.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class StaticAnalyzer {
private:
    std::vector<std::set<std::string_view>> symbol_table;
    std::unordered_map<std::string_view, std::uint64_t> function_table;

public:
    bool has_error;
    std::string error;

    void analyze(Prog::Ptr &);

    StaticAnalyzer();
};