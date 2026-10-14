#include "parser.hpp"
#include "node.hpp"
#include "token.hpp"

#include <format>
#include <optional>
#include <utility>

Parser::Parser(Lexer &lexer_) : lexer(lexer_), has_error(false), error("") { }

auto Parser::make_error(std::string err, Token &tok) -> void {
    has_error = true;
    error = std::format("{} at {}:{}", err, tok.line, tok.column);
}

auto Parser::expr() -> std::optional<Expr::Ptr> {
    return expr_bp(0);
}

auto Parser::assignable() -> std::optional<Expr::Ptr> {
    return expr_bp(100);
}

auto Parser::expr_bp(std::uint64_t min_bp) -> std::optional<Expr::Ptr> {
    Expr::Ptr return_expr;
    Token tok = lexer.next_token();

    if (tok.type == TokenType::LeftParenthesis) {
        std::optional<Expr::Ptr> paren_expr = expr();

        if (!paren_expr.has_value()) {
            return std::nullopt;
        }

        Token rparen = lexer.next_token();
        if (rparen.type != TokenType::RightParenthesis) {
            make_error("Expect right parenthesis", rparen);
            return std::nullopt;
        }

        return_expr = std::move(paren_expr.value());
    }
    else if (tok.is_oper()) {
        std::optional<std::uint64_t> result = prefix_bp(op_type_convert(tok.type));
        if (!result.has_value()) {
            make_error("Expect prefix operator", tok);
            return std::nullopt;
        }

        std::optional<Expr::Ptr> rhs = expr_bp(result.value());
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        return_expr = std::make_unique<UnaryExpr>(op_type_convert(tok.type), std::move(rhs.value()));
    }
    else if (tok.is_value()) {
        switch (tok.type) {
        case TokenType::Identifier:
            return_expr = std::make_unique<LValue>(tok.lexeme);
            break;
        case TokenType::Integer:
            return_expr = std::make_unique<ConstExpr>(ValType::Int, tok.lexeme);
            break;
        case TokenType::Floating:
            return_expr = std::make_unique<ConstExpr>(ValType::Flt, tok.lexeme);
            break;
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
        if (op.is_delim()) {
            break;
        }
        else if (!op.is_oper()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        // std::optional<std::pair<std::uint64_t, std::uint64_t>>
        auto bp_result = infix_bp(op_type_convert(op.type));
        if (!bp_result.has_value()) {
            make_error("Expect infix operator", op);
            return std::nullopt;
        }

        auto [l_bp, r_bp] = bp_result.value();
        if (l_bp < min_bp) {
            break;
        }

        (void)lexer.next_token();

        if (try_index(op, return_expr)) {
            continue;
        }
        else if (has_error) {
            return std::nullopt;
        }

        if (try_func(op, return_expr)) {
            continue;
        }
        else if (has_error) {
            return std::nullopt;
        }

        std::optional<Expr::Ptr> rhs = expr_bp(r_bp);
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        return_expr =
            std::make_unique<BinExpr>(op_type_convert(op.type), std::move(return_expr), std::move(rhs.value()));
    }

    return return_expr;
}

auto Parser::try_index(Token &op, Expr::Ptr &lhs) -> bool {
    if (op_type_convert(op.type) != OpType::Brk) {
        return false;
    }

    LValue *lvalue = dynamic_cast<LValue *>(lhs.get());

    if (lvalue == nullptr) {
        make_error("Expect indexable", op);
        return false;
    }

    if (lexer.next_token(true).type == TokenType::RightBracket) {
        (void)lexer.next_token();
        lvalue->dims.emplace_back(nullptr);
        return true;
    }

    std::optional<Expr::Ptr> index = expr();
    if (!index.has_value()) {
        return false;
    }

    Token rbracket = lexer.next_token();
    if (rbracket.type != TokenType::RightBracket) {
        make_error("Expect right bracket", rbracket);
        return false;
    }

    lvalue->dims.emplace_back(std::move(index.value()));
    return true;
}

auto Parser::try_func(Token &op, Expr::Ptr &lhs) -> bool {
    if (op_type_convert(op.type) != OpType::Prn) {
        return false;
    }

    LValue *lvalue = dynamic_cast<LValue *>(lhs.get());

    if (lvalue == nullptr || !lvalue->dims.empty()) {
        make_error("Expect callable", op);
        return false;
    }

    lhs = std::make_unique<FuncExpr>(lvalue->name);

    if (lexer.next_token(true).type == TokenType::RightParenthesis) {
        (void)lexer.next_token();
        return true;
    }

    while (true) {
        std::optional<Expr::Ptr> arg = expr();
        if (!arg.has_value()) {
            return false;
        }

        Token delim = lexer.next_token();
        if (delim.type == TokenType::Comma) {
            static_cast<FuncExpr *>(lhs.get())->args.emplace_back(std::move(arg.value()));
            continue;
        }
        else if (delim.type == TokenType::RightParenthesis) {
            static_cast<FuncExpr *>(lhs.get())->args.emplace_back(std::move(arg.value()));
            break;
        }

        make_error("Expect comma or right parenthesis", delim);
        return false;
    }

    return true;
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
    case OpType::Prn:
        return std::pair{90, 91};
        break;
    case OpType::Brk:
        return std::pair{100, 101};
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