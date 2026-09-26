#include "lexer.hpp"
#include "token.hpp"

#include <cstdint>
#include <format>
#include <string_view>

Lexer::Lexer(std::string source_) : source(source_), error(""), pos(0), line(1), column(1), has_error(false) { }

auto Lexer::next_token() -> Token {
    std::uint64_t source_len = source.length();

    if (has_error) {
        return make_error();
    }

    std::expected<void, Token> result = skip_ignored();
    if (!result.has_value()) {
        return result.error();
    }

    if (pos >= source_len) {
        return make_token(TokenType::Eof, 0);
    }

    char current = peek(0);

    if (current == '\"') {
        return str();
    }
    else if ((current == '.' && pos + 1 < source_len && is_num(peek(1))) || is_num(current)) {
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

auto Lexer::make_error() -> Token {
    return Token(TokenType::Err, std::string_view(error), line, column);
}

auto Lexer::make_error(std::string error_message) -> Token {
    has_error = true;
    error = std::format("{} at {}:{}", error_message, line, column);
    return Token(TokenType::Err, std::string_view(error), line, column);
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

auto Lexer::advance(std::uint64_t length) -> void {
    for (std::uint64_t i = 0; i < length; i++) {
        advance();
    }
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
            advance(2);
            properly_ended = true;
            break;
        }
        advance();
    }

    if (properly_ended) {
        return {};
    }

    return std::unexpected(make_error("Unterminated comment"));
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

// "([^\\"]|(\\n)|(\\t)|(\\r)|(\\\\)|(\\')|(\\")|(\\a))*"
auto Lexer::str() -> Token {
    std::uint64_t offset = 1;
    bool escaping = false;
    bool properly_ended = false;

    while (pos + offset < source.length()) {
        if (!escaping && peek(offset) == '\n') {
            break;
        }
        else if (!escaping && peek(offset) == '\"') {
            properly_ended = true;
            break;
        }
        else if (!escaping && peek(offset) == '\\') {
            escaping = true;
            offset++;
            continue;
        }
        else if (!escaping) {
            offset++;
            continue;
        }

        char c = peek(offset);
        if (!(c == 'n' || c == 't' || c == 'r' || c == '\\' || c == '\'' || c == '\"' || c == 'a')) {
            return make_error("Invalid escape sequence");
        }
        escaping = false;

        offset++;
    }

    if (!properly_ended) {
        return make_error("Unterminated string");
    }

    offset++;
    Token return_token = make_token(TokenType::String, offset);
    advance(offset);
    return return_token;
}

// ([0-9]+(\.[0-9]*)?|\.[0-9]+)((e|E)(\+|\-)?[0-9]+)?
auto Lexer::num() -> Token {
    std::uint64_t offset = 1;
    std::uint64_t state = (peek(0) == '.');
    bool is_float;

    while (pos + offset < source.length()) {
        char current = peek(offset);

        if (state == 0 && is_white(current)) {
            break;
        }
        else if (state == 0 && current == '.') {
            state = 1;
        }
        else if (state == 0 && (current == 'e' || current == 'E')) {
            state = 2;
        }
        else if (state == 0 && !is_num(current)) {
            return make_error("Invalid number");
        }

        else if (state == 1 && is_white(current)) {
            break;
        }
        else if (state == 1 && (current == 'e' || current == 'E')) {
            state = 2;
        }
        else if (state == 1 && !is_num(current)) {
            return make_error("Invalid number");
        }

        else if (state == 2 && (current == '+' || current == '-')) {
            state = 3;
        }
        else if (state == 2 && is_num(current)) {
            state = 4;
        }
        else if (state == 2) {
            return make_error("Invalid number");
        }

        else if (state == 3 && is_num(current)) {
            state = 4;
        }
        else if (state == 3) {
            return make_error("Invalid number");
        }

        else if (state == 4 && is_white(current)) {
            break;
        }
        else if (state == 4 && !is_num(current)) {
            return make_error("Invalid number");
        }

        offset++;
    }

    if (state == 0) {
        is_float = false;
    }
    else if (state == 1 || state == 4) {
        is_float = true;
    }
    else {
        return make_error("Invalid number");
    }

    Token return_token = make_token(is_float ? TokenType::Float : TokenType::Integer, offset);
    advance(offset);
    return return_token;
}

// [a-zA-Z_][a-zA-Z0-9_]*
auto Lexer::iden() -> Token {
    TokenType return_type;
    std::uint64_t offset = 1;

    while (pos + offset < source.length() && is_alnum(peek(offset))) {
        offset++;
    }

    std::string_view iden = std::string_view(source).substr(pos, offset);

    if (iden == "return") {
        return_type = TokenType::Return;
    }
    else if (iden == "typedef") {
        return_type = TokenType::Typedef;
    }
    else if (iden == "if") {
        return_type = TokenType::If;
    }
    else if (iden == "else") {
        return_type = TokenType::Else;
    }
    else if (iden == "int") {
        return_type = TokenType::Int;
    }
    else if (iden == "float") {
        return_type = TokenType::Float;
    }
    else if (iden == "for") {
        return_type = TokenType::For;
    }
    else if (iden == "struct") {
        return_type = TokenType::Struct;
    }
    else if (iden == "union") {
        return_type = TokenType::Union;
    }
    else if (iden == "void") {
        return_type = TokenType::Void;
    }
    else if (iden == "while") {
        return_type = TokenType::While;
    }
    else {
        return_type = TokenType::Identifier;
    }

    Token return_token = make_token(return_type, offset);
    advance(offset);
    return return_token;
}

// (\+|\-|\*|\/|\|\||&&|{|}|\[|\]|\(|\)|;|,|\.)|((<|>|!|=)=?)
auto Lexer::oper() -> Token {
    TokenType return_type;
    std::uint64_t source_len = source.length();
    std::uint64_t offset = 1;

    switch (peek(0)) {
    case '+':
        return_type = TokenType::Plus;
        break;
    case '-':
        return_type = TokenType::Minus;
        break;
    case '*':
        return_type = TokenType::Multiply;
        break;
    case '/':
        return_type = TokenType::Divide;
        break;
    case '<':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_type = TokenType::LesserEqual;
            offset++;
            break;
        }
        return_type = TokenType::Lesser;
        break;
    case '>':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_type = TokenType::GreaterEqual;
            offset++;
            break;
        }
        return_type = TokenType::Greater;
        break;
    case '!':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_type = TokenType::NotEqual;
            offset++;
            break;
        }
        return_type = TokenType::LogicalNot;
        break;
    case '=':
        if (pos + 1 < source_len && peek(1) == '=') {
            return_type = TokenType::Equal;
            offset++;
            break;
        }
        return_type = TokenType::Assign;
        break;
    case '|':
        if (pos + 1 < source_len && peek(1) == '|') {
            return_type = TokenType::LogicalOr;
            offset++;
            break;
        }
        return make_error("Unknown operator");
        break;
    case '&':
        if (pos + 1 < source_len && peek(1) == '&') {
            return_type = TokenType::LogicalAnd;
            offset++;
            break;
        }
        return make_error("Unknown operator");
        break;
    case '{':
        return_type = TokenType::LeftBrace;
        break;
    case '}':
        return_type = TokenType::RightBrace;
        break;
    case '[':
        return_type = TokenType::LeftBracket;
        break;
    case ']':
        return_type = TokenType::RightBracket;
        break;
    case '(':
        return_type = TokenType::LeftParenthesis;
        break;
    case ')':
        return_type = TokenType::RightParenthesis;
        break;
    case ';':
        return_type = TokenType::Semicolon;
        break;
    case ',':
        return_type = TokenType::Comma;
        break;
    case '.':
        return_type = TokenType::Dot;
        break;
    default:
        return make_error("Unknown operator");
        break;
    }

    Token return_token = make_token(return_type, offset);
    advance();
    return return_token;
}