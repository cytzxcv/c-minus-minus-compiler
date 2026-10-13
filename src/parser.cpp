#include "parser.hpp"
#include "node.hpp"
#include "token.hpp"

#include <format>
#include <optional>
#include <utility>

Parser::Parser(Lexer &lexer_) : lexer(lexer_) { }

auto Parser::make_error(std::string err, Token &tok) -> void {
    has_error = true;
    error = std::format("{} at {}:{}", err, tok.line, tok.column);
}

auto Parser::expr() -> std::optional<Expr::Ptr> {
    return expr_bp(0);
}

auto Parser::expr_bp(std::uint64_t min_bp) -> std::optional<Expr::Ptr> {
    Expr::Ptr expr;
    Token tok = lexer.next_token(false);
    if (tok.is_oper()) {
        // todo
        std::optional<std::uint64_t> result = prefix_bp(op_type_convert(tok.type));
        if (!result.has_value()) {
            make_error("Expect prefix operator", tok);
            return std::nullopt;
        }
        std::optional<Expr::Ptr> rhs = expr_bp(result.value());
        if (!rhs.has_value()) {
            return std::nullopt;
        }
        expr = std::make_unique<UnaryExpr>(op_type_convert(tok.type), std::move(rhs.value()));
    }
    else if (tok.is_value()) {
        switch (tok.type) {
        case TokenType::Identifier:
            expr = std::make_unique<LValue>(tok.lexeme);
        case TokenType::Integer:
            expr = std::make_unique<ConstExpr>(ValType::Int, tok.lexeme);
        case TokenType::Floating:
            expr = std::make_unique<ConstExpr>(ValType::Flt, tok.lexeme);
        default:
            // unreachable
        }
    }
    else {
        make_error("Expect identifier or constant", tok);
        return std::nullopt;
    }

    while (true) {
        Token op = lexer.next_token(true);
        if (op.type == TokenType::Eof) {
            break;
        }
        else if (!op.is_oper()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        // std::optional<std::pair<std::uint64_t, std::uint64_t>>
        auto result = infix_bp(op_type_convert(op.type));
        if (!result.has_value()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        auto [l_bp, r_bp] = result.value();
        if (l_bp < min_bp) {
            break;
        }

        (void)lexer.next_token();
        std::optional<Expr::Ptr> rhs = expr_bp(r_bp);
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        expr = std::make_unique<BinExpr>(op_type_convert(op.type), std::move(expr), std::move(rhs.value()));
    }

    return expr;
}

auto Parser::infix_bp(OpType op) -> std::optional<std::pair<std::uint64_t, std::uint64_t>> {
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

    return std::nullopt;
}

auto Parser::prefix_bp(OpType op) -> std::optional<std::uint64_t> {
    switch (op) {
    case OpType::Add:
    case OpType::Sub:
    case OpType::Not:
        return 80;
    default:
    }

    return std::nullopt;
}