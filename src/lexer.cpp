#include "lexer.hpp"
#include "token.hpp"

#include <cstdint>
#include <string_view>

auto Lexer::next() -> Token {
    std::expected<void, Token> result = skip_ignored();
    if (!result.has_value()) {
        return result.error();
    }

    if (pos >= source.length()) {
        return Token(TokenType::Eof, std::string_view(source).substr(0, 0), line, column);
    }

    char current = source[pos];

    if (current == '\"') {
        return str();
    }
    else if (is_num(current)) {
        return num();
    }
    else if (is_alpha(current)) {
        return iden();
    }
    return oper();
}

auto Lexer::advance() -> void {
    if (source[pos] == '\n') {
        column = 1;
        line++;
    }
    else {
        column++;
    }
    pos++;
}

auto Lexer::skip_single() -> void {
    while (pos < source.length() && source[pos] != '\n') {
        advance();
    }
}

auto Lexer::skip_multi() -> std::expected<void, Token> {
    while (pos + 1 < source.length() && !(source[pos] == '*' && source[pos + 1] == '/')) {
        advance();
    }

    if (pos >= source.length()) {
        return std::unexpected(Token(TokenType::Err, std::string_view(source).substr(0, 0), line, column));
    }

    return {};
}

auto Lexer::skip_ignored() -> std::expected<void, Token> {
    uint64_t source_len = source.length();

    while (true) {
        if (pos >= source_len) {
            break;
        }

        while (pos < source_len && is_white(source[pos])) {
            advance();
        }

        if (pos + 1 < source_len && source[pos] == '/' && source[pos + 1] == '/') {
            skip_single();
            continue;
        }
        else if (pos + 1 < source_len && source[pos] == '/' && source[pos + 1] == '*') {
            std::expected<void, Token> result = skip_multi();
            if (!result.has_value()) {
                return result;
            }
            continue;
        }

        break;
    }

    return {};
}

auto Lexer::is_white(char c) -> bool {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

auto Lexer::is_alpha(char c) -> bool {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

auto Lexer::is_num(char c) -> bool {
    return '0' <= c && c <= '9';
}

auto Lexer::is_alnum(char c) -> bool {
    return is_alpha(c) || is_num(c);
}