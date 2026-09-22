#include "token.hpp"

#include <cstdint>
#include <print>
#include <string_view>
#include <utility>

constexpr std::string_view token_type_to_string(TokenType type) {
    switch (type) {
    case TokenType::Return:
        return "Return";
        break;
    case TokenType::Typedef:
        return "Typedef";
        break;
    case TokenType::Struct:
        return "Struct";
        break;
    case TokenType::Union:
        return "Union";
        break;
    case TokenType::If:
        return "If";
        break;
    case TokenType::Else:
        return "Else";
        break;
    case TokenType::For:
        return "For";
        break;
    case TokenType::While:
        return "While";
        break;
    case TokenType::Int:
        return "Int";
        break;
    case TokenType::Float:
        return "Float";
        break;
    case TokenType::Void:
        return "Void";
        break;
    case TokenType::Integer:
        return "Integer";
        break;
    case TokenType::Floating:
        return "Floating";
        break;
    case TokenType::String:
        return "String";
        break;
    case TokenType::Identifier:
        return "Identifier";
        break;
    case TokenType::Plus:
        return "Plus";
        break;
    case TokenType::Minus:
        return "Minus";
        break;
    case TokenType::Multiply:
        return "Multiply";
        break;
    case TokenType::Divide:
        return "Divide";
        break;
    case TokenType::Greater:
        return "Greater";
        break;
    case TokenType::Lesser:
        return "Lesser";
        break;
    case TokenType::GreaterEqual:
        return "GreaterEqual";
        break;
    case TokenType::LesserEqual:
        return "LesserEqual";
        break;
    case TokenType::Equal:
        return "Equal";
        break;
    case TokenType::NotEqual:
        return "NotEqual";
        break;
    case TokenType::LogicalOr:
        return "LogicalOr";
        break;
    case TokenType::LogicalAnd:
        return "LogicalAnd";
        break;
    case TokenType::LogicalNot:
        return "LogicalNot";
        break;
    case TokenType::Assign:
        return "Assign";
        break;
    case TokenType::Semicolon:
        return "Semicolon";
        break;
    case TokenType::Comma:
        return "Comma";
        break;
    case TokenType::Dot:
        return "Dot";
        break;
    case TokenType::LeftParen:
        return "LeftParen";
        break;
    case TokenType::RightParen:
        return "RightParen";
        break;
    case TokenType::LeftBracket:
        return "LeftBracket";
        break;
    case TokenType::RightBracket:
        return "RightBracket";
        break;
    case TokenType::LeftBrace:
        return "LeftBrace";
        break;
    case TokenType::RightBrace:
        return "RightBrace";
        break;
    case TokenType::Eof:
        return "Eof";
        break;
    case TokenType::Err:
        return "Err";
        break;
    default:
        std::unreachable();
    }
}

Token::Token(TokenType type_, std::string_view lexeme_, std::uint64_t line_, std::uint64_t column_)
    : type(type_), lexeme(lexeme_), line(line_), column(column_) { }

auto Token::print() -> void {
    std::println("Token({}, {:?}, {}, {})", token_type_to_string(type), lexeme, line, column);
}