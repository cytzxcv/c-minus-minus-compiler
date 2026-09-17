#pragma once

#include "token.hpp"

#include <cstdint>
#include <string>

class Lexer {
private:
    std::string source;
    std::uint64_t pos, line, column;

    bool is_num(char);

public:
    Lexer(std::string source) : source(source), pos(0), line(1), column(1) { }
    Token next();
};