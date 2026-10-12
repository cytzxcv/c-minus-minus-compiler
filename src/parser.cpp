#include "parser.hpp"
#include "token.hpp"

#include <format>
#include <optional>
#include <utility>

Parser::Parser(Lexer &lexer_) : lexer(lexer_) { }

auto Parser::make_error(std::string err, Token &tok) -> void {
    error = std::format("{} at {}:{}", err, tok.line, tok.column);
}

auto Parser::ttype_to_otype(TokenType tok_type) -> OpType {
    switch (tok_type) {
    case TokenType::Plus:
        return OpType::Add;
        break;
    case TokenType::Minus:
        return OpType::Sub;
        break;
    case TokenType::Multiply:
        return OpType::Mul;
        break;
    case TokenType::Divide:
        return OpType::Div;
        break;
    case TokenType::Lesser:
        return OpType::Lt;
        break;
    case TokenType::Greater:
        return OpType::Gt;
        break;
    case TokenType::LesserEqual:
        return OpType::Leq;
        break;
    case TokenType::GreaterEqual:
        return OpType::Geq;
        break;
    case TokenType::Equal:
        return OpType::Eq;
        break;
    case TokenType::NotEqual:
        return OpType::Neq;
        break;
    case TokenType::LogicalOr:
        return OpType::Or;
        break;
    case TokenType::LogicalAnd:
        return OpType::And;
        break;
    case TokenType::LogicalNot:
        return OpType::Not;
        break;
    case TokenType::Assign:
        return OpType::Mov;
        break;
    default:
    }

    // unreachable
    return OpType::Mov;
}

auto Parser::expr() -> std::optional<std::unique_ptr<Expr>> {
    return expr_bp(0);
}

auto Parser::expr_bp(uint64_t min_bp) -> std::optional<std::unique_ptr<Expr>> {
    // todo
    (void)min_bp;
    Token lhs = lexer.next_token();
    if (!lhs.is_value()) {
        make_error("Expect identifier or constant", lhs);
        return std::nullopt;
    }

    std::unique_ptr<Expr> expr;
    switch (lhs.type) {
    case TokenType::Identifier:
        expr = std::make_unique<LValue>(lhs.lexeme);
    case TokenType::Integer:
        expr = std::make_unique<ConstExpr>(ValType::Int, lhs.lexeme);
    case TokenType::Floating:
        expr = std::make_unique<ConstExpr>(ValType::Flt, lhs.lexeme);
    default:
        // unreachable
    }

    while (true) {
        Token op = lexer.next_token();
        if (op.type == TokenType::Eof) {
            break;
        }
        else if (!op.is_oper()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        // std::optional<std::pair<uint64_t, uint64_t>>
        auto result = infix_bp(ttype_to_otype(op.type));
        if (!result.has_value()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        auto [l_bp, r_bp] = result.value();
        if (l_bp < min_bp) {
            break;
        }

        std::optional<std::unique_ptr<Expr>> rhs = expr_bp(r_bp);
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        expr = std::make_unique<BinExpr>(ttype_to_otype(op.type), std::move(expr), std::move(rhs.value()));
    }

    return expr;
}

auto Parser::infix_bp(OpType op) -> std::optional<std::pair<uint64_t, uint64_t>> {
    switch (op) {
    case OpType::Mov:
        return std::pair{11, 10};
        break;
    case OpType::Or:
        return std::pair{20, 21};
        break;
    case OpType::And:
        return std::pair{30, 31};
        break;
    case OpType::Eq:
    case OpType::Neq:
        return std::pair{40, 41};
        break;
    case OpType::Lt:
    case OpType::Gt:
    case OpType::Leq:
    case OpType::Geq:
        return std::pair{50, 51};
        break;
    case OpType::Add:
    case OpType::Sub:
        return std::pair{60, 61};
        break;
    case OpType::Mul:
    case OpType::Div:
        return std::pair{70, 71};
        break;
    default:
    }

    // logical not
    return std::nullopt;
}