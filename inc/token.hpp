#pragma once

#include <cstdint>
#include <string_view>

// clang-format off
enum class TokenType {
    Return, Typedef, Struct, Union,
    If, Else, For, While,
    Int, Float, Void,
    Integer, Floating, String, Identifier,
    Plus, Minus, Multiply, Divide,
    Greater, Lesser, GreaterEqual, LesserEqual, Equal, NotEqual, 
    LogicalOr, LogicalAnd, LogicalNot,
    Assign,
    Semicolon, Comma, Dot,
    LeftParen, RightParen, LeftBracket, RightBracket, LeftBrace, RightBrace,
    Eof, Err
};
// clang-format on

class Token {
public:
    TokenType type;
    std::string_view lexeme;
    std::uint64_t line, column;

    Token(TokenType type, std::string_view lexeme, std::uint64_t line, std::uint64_t column)
        : type(type), lexeme(lexeme), line(line), column(column) { }
};