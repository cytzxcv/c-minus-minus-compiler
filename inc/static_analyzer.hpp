#pragma once

#include "node.hpp"

#include <string>

class StaticAnalyzer {
private:
public:
    bool has_error;
    std::string error;

    void analyze(Prog::Ptr &);
};