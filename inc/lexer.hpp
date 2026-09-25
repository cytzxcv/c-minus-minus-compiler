#pragma once

#include "token.hpp"

#include <cstdint>
#include <expected>
#include <string>

class Lexer {
private:
    std::string source, error;
    std::uint64_t pos, line, column;
    bool has_error;

    auto make_token(TokenType, std::uint64_t) -> Token;
    auto make_error() -> Token;
    auto make_error(std::string) -> Token;

    auto peek(std::uint64_t) -> char;
    auto advance() -> void;
    auto advance(std::uint64_t) -> void;

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
    Lexer(std::string);

    auto next_token() -> Token;
};