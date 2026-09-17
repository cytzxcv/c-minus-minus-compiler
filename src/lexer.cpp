#include "lexer.hpp"
#include "token.hpp"

#include <cstdint>

Token Lexer::next() {
    std::uint64_t start = pos;
    char current = source[pos];

    // todo
}

bool Lexer::is_num(char c) {
    return '0' <= c && c <= '9';
}