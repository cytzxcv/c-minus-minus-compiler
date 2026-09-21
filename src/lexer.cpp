#include "lexer.hpp"
#include "token.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

auto Lexer::next_token() -> Token {
    std::expected<void, Token> result = skip_ignored();
    if (!result.has_value()) {
        return result.error();
    }

    if (pos >= source.length()) {
        return make_err();
    }

    char current = peek();

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

auto Lexer::make_token(TokenType type, std::uint64_t len) -> Token {
    return Token(type, std::string_view(source).substr(pos, len), line, column);
}

auto Lexer::make_err() -> Token {
    return Token(TokenType::Err, std::string_view(source).substr(0, 0), line, column);
}

auto Lexer::peek() -> char {
    return source[pos];
}

auto Lexer::peek_next() -> char {
    return source[pos + 1];
}

auto Lexer::advance() -> void {
    if (peek() == '\n') {
        column = 1;
        line++;
    }
    else {
        column++;
    }
    pos++;
}

auto Lexer::skip_single() -> void {
    while (pos < source.length() && peek() != '\n') {
        advance();
    }
}

auto Lexer::skip_multi() -> std::expected<void, Token> {
    bool properly_ended = false;

    while (pos < source.length()) {
        if (pos + 1 < source.length() && peek() == '*' && peek_next() == '/') {
            advance();
            advance();
            properly_ended = true;
            break;
        }
        advance();
    }

    if (properly_ended) {
        return {};
    }

    return std::unexpected(make_err());
}

auto Lexer::skip_ignored() -> std::expected<void, Token> {
    std::uint64_t source_len = source.length();

    while (true) {
        if (pos >= source_len) {
            break;
        }

        while (pos < source_len && is_white(peek())) {
            advance();
        }

        if (pos + 1 < source_len && peek() == '/' && peek_next() == '/') {
            skip_single();
            continue;
        }
        else if (pos + 1 < source_len && peek() == '/' && peek_next() == '*') {
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

auto Lexer::str() -> Token {
    // todo
}

auto Lexer::num() -> Token {
    // todo
}

auto Lexer::iden() -> Token {
    // todo
}

auto Lexer::oper() -> Token {
    std::optional<Token> return_token = std::nullopt;
    std::uint64_t source_len = source.length();

    switch (peek()) {
    case '+':
        return_token = make_token(TokenType::Plus, 1);
        break;
    case '-':
        return_token = make_token(TokenType::Minus, 1);
        break;
    case '*':
        return_token = make_token(TokenType::Multiply, 1);
        break;
    case '/':
        return_token = make_token(TokenType::Divide, 1);
        break;
    case '<':
        if (pos + 1 < source_len && peek_next() == '=') {
            return_token = make_token(TokenType::LesserEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::Lesser, 1);
        break;
    case '>':
        if (pos + 1 < source_len && peek_next() == '=') {
            return_token = make_token(TokenType::GreaterEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::Greater, 1);
        break;
    case '!':
        if (pos + 1 < source_len && peek_next() == '=') {
            return_token = make_token(TokenType::NotEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::LogicalNot, 1);
        break;
    case '=':
        if (pos + 1 < source_len && peek_next() == '=') {
            return_token = make_token(TokenType::Equal, 2);
            advance();
        }
        return_token = make_token(TokenType::Assign, 1);
        break;
    case '|':
        if (pos + 1 < source_len && peek_next() == '|') {
            return_token = make_token(TokenType::LogicalOr, 2);
            advance();
        }
        return_token = make_err();
        break;
    case '&':
        if (pos + 1 < source_len && peek_next() == '&') {
            return_token = make_token(TokenType::LogicalAnd, 2);
            advance();
        }
        return_token = make_err();
        break;
    case '{':
        return_token = make_token(TokenType::LeftBrace, 1);
        break;
    case '}':
        return_token = make_token(TokenType::RightBrace, 1);
        break;
    case '[':
        return_token = make_token(TokenType::LeftBracket, 1);
        break;
    case ']':
        return_token = make_token(TokenType::RightBracket, 1);
        break;
    case '(':
        return_token = make_token(TokenType::LeftParen, 1);
        break;
    case ')':
        return_token = make_token(TokenType::RightParen, 1);
        break;
    case ';':
        return_token = make_token(TokenType::Semicolon, 1);
        break;
    case ',':
        return_token = make_token(TokenType::Comma, 1);
        break;
    case '.':
        return_token = make_token(TokenType::Dot, 1);
        break;
    default:
        return_token = make_err();
        break;
    }

    advance();
    return return_token.value();
}