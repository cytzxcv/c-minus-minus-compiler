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

auto Parser::expr() -> std::optional<std::unique_ptr<Expr>> {
    return expr_bp(0);
}

auto Parser::expr_bp(uint64_t min_bp) -> std::optional<std::unique_ptr<Expr>> {
    Token lhs = lexer.next_token(false);
    // lhs.print();
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
        Token op = lexer.next_token(true);
        // op.print();
        if (op.type == TokenType::Eof) {
            break;
        }
        else if (!op.is_oper()) {
            make_error("Expect infix operator1", op);
            return std::nullopt;
        }

        // std::optional<std::pair<uint64_t, uint64_t>>
        auto result = infix_bp(op_type_convert(op.type));
        if (!result.has_value()) {
            make_error("Expect infix operator2", op);
            return std::nullopt;
        }

        auto [l_bp, r_bp] = result.value();
        if (l_bp < min_bp) {
            break;
        }

        (void)lexer.next_token();
        std::optional<std::unique_ptr<Expr>> rhs = expr_bp(r_bp);
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        expr = std::make_unique<BinExpr>(op_type_convert(op.type), std::move(expr), std::move(rhs.value()));
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