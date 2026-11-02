#pragma once

#include "token.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <utility>

class Lexer {
private:
    std::string source;
    std::uint64_t pos, line, column;

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

    auto str() -> std::pair<Token, std::uint64_t>;
    auto num() -> std::pair<Token, std::uint64_t>;
    auto iden() -> std::pair<Token, std::uint64_t>;
    auto oper() -> std::pair<Token, std::uint64_t>;

public:
    bool has_error;
    std::string error;

    Lexer(std::string);

    auto next_token(bool is_peeking = false) -> Token;
};