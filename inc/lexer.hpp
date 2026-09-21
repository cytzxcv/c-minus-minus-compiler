#pragma once

#include "token.hpp"

#include <cstdint>
#include <expected>
#include <string>

class Lexer {
private:
    std::string source;
    std::uint64_t pos, line, column;

    auto make_token(TokenType, std::uint64_t) -> Token;
    auto make_err() -> Token;

    auto peek() -> char;
    auto peek_next() -> char;
    auto advance() -> void;

    auto skip_single() -> void;
    auto skip_multi() -> std::expected<void, Token>;
    auto skip_ignored() -> std::expected<void, Token>;

    auto is_white(char) -> bool;
    auto is_alpha(char) -> bool;
    auto is_num(char) -> bool;
    auto is_alnum(char) -> bool;

    auto str() -> Token;
    auto num() -> Token;
    auto iden() -> Token;
    auto oper() -> Token;

public:
    Lexer(std::string source) : source(source), pos(0), line(1), column(1) { }

    auto next_token() -> Token;
};