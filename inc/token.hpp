#pragma once

#include <cstdint>
#include <string_view>

enum class TokenType {
    Return,
    Typedef,
    If,
    Else,
    For,
    While,
    Int,
    Float,
    Void,
    Integer,
    Floating,
    String,
    Identifier,
    Plus,
    Minus,
    Multiply,
    Divide,
    Greater,
    Lesser,
    GreaterEqual,
    LesserEqual,
    Equal,
    NotEqual,
    LogicalOr,
    LogicalAnd,
    LogicalNot,
    Assign,
    Semicolon,
    Comma,
    Dot,
    LeftParenthesis,
    RightParenthesis,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Eof,
    Err,
};

constexpr std::string_view token_type_to_string(TokenType type);

class Token {
public:
    TokenType type;
    std::string_view lexeme;
    std::uint64_t line, column;

    Token(TokenType, std::string_view, std::uint64_t, std::uint64_t);
    auto print() -> void;

    auto is_value() -> bool;
    auto is_oper() -> bool;
    auto is_delim() -> bool;
};