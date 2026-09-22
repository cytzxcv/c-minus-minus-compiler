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

    char current = peek(0);

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

auto Lexer::peek(std::uint64_t offset) -> char {
    return source[pos + offset];
}

auto Lexer::advance() -> void {
    if (peek(0) == '\n') {
        column = 1;
        line++;
    }
    else {
        column++;
    }
    pos++;
}

auto Lexer::skip_single() -> void {
    while (pos < source.length() && peek(0) != '\n') {
        advance();
    }
}

auto Lexer::skip_multi() -> std::expected<void, Token> {
    bool properly_ended = false;

    while (pos < source.length()) {
        if (pos + 1 < source.length() && peek(0) == '*' && peek(1) == '/') {
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

        while (pos < source_len && is_white(peek(0))) {
            advance();
        }

        if (pos + 1 < source_len && peek(0) == '/' && peek(1) == '/') {
            skip_single();
            continue;
        }
        else if (pos + 1 < source_len && peek(0) == '/' && peek(1) == '*') {
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
    std::uint64_t offset = 0;
    bool escaping = false;

    while (pos + offset < source.length()) {
        offset++;

        if (!escaping && peek(offset) == '\"') {
            break;
        }
        else if (!escaping && peek(offset) == '\\') {
            escaping = true;
            continue;
        }
        else if (!escaping) {
            continue;
        }

        char c = peek(offset);
        if (!(c == 'n' || c == 't' || c == 'r' || c == '\\' || c == '\'' || c == '\"' || c == 'a')) {
            return make_err();
        }
        escaping = false;
    }

    Token return_token = make_token(TokenType::String, offset);
    for (std::uint64_t i = 0; i < offset; i++) {
        advance();
    }
    return return_token;
}

auto Lexer::num() -> Token {
    // todo
}

auto Lexer::iden() -> Token {
    std::optional<Token> return_token = std::nullopt;
    std::uint64_t offset = 0;

    while (pos + offset < source.length() && is_alnum(peek(offset))) {
        offset++;
    }

    std::string_view iden = std::string_view(source).substr(pos, offset);

    if (iden == "return") {
        return_token = make_token(TokenType::Return, offset);
    }
    else if (iden == "typedef") {
        return_token = make_token(TokenType::Typedef, offset);
    }
    else if (iden == "if") {
        return_token = make_token(TokenType::If, offset);
    }
    else if (iden == "else") {
        return_token = make_token(TokenType::Else, offset);
    }
    else if (iden == "int") {
        return_token = make_token(TokenType::Int, offset);
    }
    else if (iden == "float") {
        return_token = make_token(TokenType::Float, offset);
    }
    else if (iden == "for") {
        return_token = make_token(TokenType::For, offset);
    }
    else if (iden == "struct") {
        return_token = make_token(TokenType::Struct, offset);
    }
    else if (iden == "union") {
        return_token = make_token(TokenType::Union, offset);
    }
    else if (iden == "void") {
        return_token = make_token(TokenType::Void, offset);
    }
    else if (iden == "while") {
        return_token = make_token(TokenType::While, offset);
    }
    else {
        return_token = make_token(TokenType::Int, offset);
    }

    for (std::uint64_t i = 0; i < offset; i++) {
        advance();
    }
    return return_token.value();
}

auto Lexer::oper() -> Token {
    std::optional<Token> return_token = std::nullopt;
    std::uint64_t source_len = source.length();

    switch (peek(0)) {
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
        if (pos + 1 < source_len && peek(1) == '=') {
            return_token = make_token(TokenType::LesserEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::Lesser, 1);
        break;
    case '>':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_token = make_token(TokenType::GreaterEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::Greater, 1);
        break;
    case '!':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_token = make_token(TokenType::NotEqual, 2);
            advance();
        }
        return_token = make_token(TokenType::LogicalNot, 1);
        break;
    case '=':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_token = make_token(TokenType::Equal, 2);
            advance();
        }
        return_token = make_token(TokenType::Assign, 1);
        break;
    case '|':
        if (pos + 1 < source_len && peek(1) == '|') {
            return_token = make_token(TokenType::LogicalOr, 2);
            advance();
        }
        return_token = make_err();
        break;
    case '&':
        if (pos + 1 < source_len && peek(1) == '&') {
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