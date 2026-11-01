#include "parser.hpp"
#include "node.hpp"
#include "token.hpp"

#include <format>
#include <optional>
#include <utility>

Parser::Parser(Lexer &lexer_) : lexer(lexer_), type_table(), has_error(false), error("") {
    type_table.emplace_back();
}

auto Parser::make_error(std::string err, Token &tok) -> void {
    has_error = true;
    error = std::format("{} at {}:{}", err, tok.line, tok.column);
}

auto Parser::block_item() -> BlockStmt::Item {
    TokenType look_ahead = lexer.next_token(true).type;

    if (look_ahead == TokenType::Typedef) {
        return type_decl();
    }
    else if (look_ahead == TokenType::If) {
        return if_stmt();
    }
    else {
        return expr_stmt();
    }

    return static_cast<Decl::Ptr>(nullptr);
}

// decl
auto Parser::type_decl() -> Decl::Ptr {
    lexer.next_token(); // consume "typedef"

    ValType type;
    Token type_token = lexer.next_token();

    if (type_token.type == TokenType::Int) {
        type = ValType::Int;
    }
    else if (type_token.type == TokenType::Float) {
        type = ValType::Flt;
    }
    else if (type_token.type == TokenType::Identifier && type_table.back().contains(type_token.lexeme)) {
        type = type_table.back()[type_token.lexeme];
    }
    else {
        make_error("Expect valid type", type_token);
        return nullptr;
    }

    Token name_token = lexer.next_token();
    if (name_token.type != TokenType::Identifier) {
        make_error("Expect identifier", name_token);
        return nullptr;
    }
    else if (type_table.back().contains(name_token.lexeme)) {
        make_error("Expect undefined symbol", name_token);
        return nullptr;
    }

    type_table.back()[name_token.lexeme] = type;
    return make_unique<TypeDecl>(type, name_token.lexeme);
}

// stmt
auto Parser::expr_stmt() -> Stmt::Ptr {
    return std::make_unique<ExprStmt>(expr());
}

auto Parser::if_stmt() -> Stmt::Ptr { }

// expr
auto Parser::expr() -> Expr::Ptr {
    return expr_bp(0);
}

auto Parser::assignable() -> Expr::Ptr {
    return expr_bp(100);
}

auto Parser::expr_bp(std::uint64_t min_bp) -> Expr::Ptr {
    Expr::Ptr return_expr;
    Token tok = lexer.next_token();

    if (tok.type == TokenType::LeftParenthesis) {
        std::optional<Expr::Ptr> paren_expr = expr();

        if (!paren_expr.has_value()) {
            return nullptr;
        }

        Token rparen = lexer.next_token();
        if (rparen.type != TokenType::RightParenthesis) {
            make_error("Expect right parenthesis", rparen);
            return nullptr;
        }

        return_expr = std::move(paren_expr.value());
    }
    else if (tok.is_oper()) {
        std::optional<std::uint64_t> result = prefix_bp(op_type_convert(tok.type));
        if (!result.has_value()) {
            make_error("Expect prefix operator", tok);
            return nullptr;
        }

        std::optional<Expr::Ptr> rhs = expr_bp(result.value());
        if (!rhs.has_value()) {
            return nullptr;
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
        return nullptr;
    }

    while (true) {
        Token op = lexer.next_token(true);
        if (op.is_delim()) {
            break;
        }
        else if (!op.is_oper()) {
            make_error("Expect infix operator", op);
            return nullptr;
        }

        // std::optional<std::pair<std::uint64_t, std::uint64_t>>
        auto bp_result = infix_bp(op_type_convert(op.type));
        if (!bp_result.has_value()) {
            make_error("Expect infix operator", op);
            return nullptr;
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
            return nullptr;
        }

        if (try_func(op, return_expr)) {
            continue;
        }
        else if (has_error) {
            return nullptr;
        }

        std::optional<Expr::Ptr> rhs = expr_bp(r_bp);
        if (!rhs.has_value()) {
            return nullptr;
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