#include "lexer.hpp"
#include "token.hpp"

#include <cstdint>

Token Lexer::next() {
    std::uint64_t start = pos;
    char current = source[pos];

    // todo
}

bool Lexer::is_alpha(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

bool Lexer::is_num(char c) {
    return '0' <= c && c <= '9';
}

bool Lexer::is_alnum(char c) {
    return is_alpha(c) || is_num(c);
}