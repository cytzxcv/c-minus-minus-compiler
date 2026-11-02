#include "parser.hpp"
#include "node.hpp"
#include "token.hpp"

#include <format>
#include <optional>
#include <utility>
#include <variant>

Parser::Parser(Lexer &lexer_) : lexer(lexer_), type_table(), has_error(false), error("") {
    type_table.emplace_back();
}

auto Parser::make_error(std::string err, Token &tok) -> void {
    has_error = true;
    error = std::format("{} at {}:{}", err, tok.line, tok.column);
}

auto Parser::block_item() -> BlockStmt::Item {
    Token look_ahead = lexer.next_token(true);

    if (look_ahead.type == TokenType::Typedef || get_type(look_ahead).has_value()) {
        return decl();
    }

    return stmt();
}

// decl
auto Parser::decl() -> Decl::Ptr {
    TokenType look_ahead = lexer.next_token(true).type;

    switch (look_ahead) {
    case TokenType::Typedef:
        return type_decl();
        break;
    default:
        return var_func_decl();
        break;
    }
}

auto Parser::type_decl() -> Decl::Ptr {
    lexer.next_token(); // consume "typedef"

    ValType type;
    Token type_token = lexer.next_token();

    std::optional<ValType> check_type = get_type(type_token);

    if (check_type.has_value()) {
        type = check_type.value();
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

    Token semicolon = lexer.next_token();
    if (semicolon.type != TokenType::Semicolon) {
        make_error("Expect semicolon", semicolon);
        return nullptr;
    }

    type_table.back()[name_token.lexeme] = type;
    return make_unique<TypeDecl>(type, name_token.lexeme);
}

auto Parser::var_func_decl() -> Decl::Ptr {
    Token type_token = lexer.next_token();
    std::optional<ValType> check_type = get_type(type_token);

    if (!check_type) {
        make_error("Expect valid type", type_token);
        return nullptr;
    }
    ValType type = check_type.value();

    LValue::Ptr lvalue = assignable();
    if (!lvalue) {
        return nullptr;
    }

    if (lvalue->dims.empty() && lexer.next_token(true).type == TokenType::LeftParenthesis) {
        return func_decl(type, lvalue->name);
    }

    return var_decl(type, std::move(lvalue));
}

auto Parser::var_decl(ValType type, LValue::Ptr first_lvalue) -> Decl::Ptr {
    std::unique_ptr<VarDecl> return_decl = std::make_unique<VarDecl>(type);
    bool var_added = false;

    Token delim = lexer.next_token();
    if (delim.type == TokenType::Assign) {
        Expr::Ptr init_expr = expr();
        if (!init_expr) {
            return nullptr;
        }

        return_decl->vars.emplace_back(std::make_pair(std::move(first_lvalue), std::move(init_expr)));
        var_added = true;

        delim = lexer.next_token();
    }

    if (!var_added) {
        return_decl->vars.emplace_back(std::make_pair(std::move(first_lvalue), nullptr));
    }

    if (delim.type == TokenType::Semicolon) {
        return return_decl;
    }
    else if (delim.type != TokenType::Comma) {
        make_error("Expect assignment or comma or semicolon", delim);
        return nullptr;
    }

    Token look_ahead = lexer.next_token(true);
    while (look_ahead.type != TokenType::Semicolon) {
        var_added = false;

        LValue::Ptr lvalue = assignable();
        if (!lvalue) {
            return nullptr;
        }

        look_ahead = lexer.next_token();

        if (look_ahead.type == TokenType::Assign) {
            Expr::Ptr init_expr = expr();
            if (!init_expr) {
                return nullptr;
            }

            return_decl->vars.emplace_back(std::make_pair(std::move(lvalue), std::move(init_expr)));
            var_added = true;

            look_ahead = lexer.next_token();
        }

        if (!var_added) {
            return_decl->vars.emplace_back(std::make_pair(std::move(lvalue), nullptr));
        }

        if (look_ahead.type != TokenType::Comma && look_ahead.type != TokenType::Semicolon) {
            make_error("Expect assignment or comma or semicolon", delim);
            return nullptr;
        }
    }

    return return_decl;
}

auto Parser::func_decl(ValType type, std::string_view name) -> Decl::Ptr {
    Token left_paren = lexer.next_token();

    if (type_table.size() > 1) {
        make_error("Function definition is only allowed for global scope", left_paren);
    }

    std::unique_ptr<FuncDecl> return_decl = std::make_unique<FuncDecl>(type, name);

    Token look_ahead = lexer.next_token(true);
    while (look_ahead.type != TokenType::RightParenthesis) {
        Token param_type_token = lexer.next_token();
        std::optional<ValType> param_type = get_type(param_type_token);
        if (!param_type) {
            make_error("Expect valid type", param_type_token);
            return nullptr;
        }

        LValue::Ptr lvalue = assignable();
        if (!lvalue) {
            return nullptr;
        }
        lvalue->is_param = true;

        look_ahead = lexer.next_token();
        if (look_ahead.type != TokenType::Comma && look_ahead.type != TokenType::RightParenthesis) {
            make_error("Expect comma or right parenthesis", look_ahead);
        }

        return_decl->params.emplace_back(std::make_pair(param_type.value(), std::move(lvalue)));
    }

    if (return_decl->params.empty()) {
        lexer.next_token(); // consume ")"
    }

    Token left_brace = lexer.next_token(true);
    if (left_brace.type != TokenType::LeftBrace) {
        make_error("Expect left brace", left_brace);
        return nullptr;
    }

    BlockStmt::Ptr body = block_stmt();
    if (!body) {
        return nullptr;
    }

    return_decl->body = std::move(body);

    return return_decl;
}

auto Parser::get_type(Token token) -> std::optional<ValType> {
    if (token.type == TokenType::Void) {
        return ValType::Void;
    }

    if (token.type == TokenType::Int) {
        return ValType::Int;
    }

    if (token.type == TokenType::Float) {
        return ValType::Flt;
    }

    if (token.type != TokenType::Identifier) {
        return std::nullopt;
    }

    for (std::size_t i = type_table.size(); i > 0; i--) {
        auto iter = type_table[i - 1].find(token.lexeme);

        if (iter != type_table[i - 1].end()) {
            return iter->second;
        }
    }

    return std::nullopt;
}

// stmt
auto Parser::stmt() -> Stmt::Ptr {
    TokenType look_ahead = lexer.next_token(true).type;

    switch (look_ahead) {
    case TokenType::LeftBrace:
        return block_stmt();
        break;
    case TokenType::If:
        return if_stmt();
        break;
    case TokenType::While:
        return while_stmt();
        break;
    case TokenType::Return:
        return return_stmt();
        break;
    default:
        return expr_stmt();
        break;
    }
}

auto Parser::block_stmt() -> BlockStmt::Ptr {
    lexer.next_token(); // consume "{"

    BlockStmt::Ptr block = std::make_unique<BlockStmt>();

    type_table.emplace_back();
    TokenType look_ahead = lexer.next_token(true).type;

    while (look_ahead != TokenType::RightBrace) {
        BlockStmt::Item item = block_item();
        if (std::visit([](const auto &ptr) { return !ptr; }, item)) {
            return nullptr;
        }

        block->items.emplace_back(std::move(item));

        look_ahead = lexer.next_token(true).type;
    }

    lexer.next_token(); // consume "}"

    type_table.pop_back();
    return block;
}

auto Parser::expr_stmt() -> Stmt::Ptr {
    Expr::Ptr return_expr = expr();
    if (!return_expr) {
        return nullptr;
    }

    Token semicolon = lexer.next_token();
    if (semicolon.type != TokenType::Semicolon) {
        make_error("Expect semicolon", semicolon);
        return nullptr;
    }

    return std::make_unique<ExprStmt>(std::move(return_expr));
}

auto Parser::if_stmt() -> Stmt::Ptr {
    lexer.next_token(); // consume "if"

    Token l_paren = lexer.next_token();
    if (l_paren.type != TokenType::LeftParenthesis) {
        make_error("Expect left parenthesis", l_paren);
        return nullptr;
    }

    Expr::Ptr cond = expr();
    if (!cond) {
        return nullptr;
    }

    Token r_paren = lexer.next_token();
    if (r_paren.type != TokenType::RightParenthesis) {
        make_error("Expect right parenthesis", r_paren);
        return nullptr;
    }

    Stmt::Ptr if_body = stmt();
    if (!if_body) {
        return nullptr;
    }

    if (lexer.next_token(true).type != TokenType::Else) {
        return std::make_unique<IfStmt>(std::move(cond), std::move(if_body), nullptr);
    }

    lexer.next_token(); // consume "else"

    Stmt::Ptr else_body = stmt();
    if (!else_body) {
        return nullptr;
    }

    return std::make_unique<IfStmt>(std::move(cond), std::move(if_body), std::move(else_body));
}

auto Parser::for_stmt() -> Stmt::Ptr { }

auto Parser::while_stmt() -> Stmt::Ptr {
    lexer.next_token(); // consume "while"

    Token l_paren = lexer.next_token();
    if (l_paren.type != TokenType::LeftParenthesis) {
        make_error("Expect left parenthesis", l_paren);
        return nullptr;
    }

    Expr::Ptr cond = expr();
    if (!cond) {
        return nullptr;
    }

    Token r_paren = lexer.next_token();
    if (r_paren.type != TokenType::RightParenthesis) {
        make_error("Expect right parenthesis", r_paren);
        return nullptr;
    }

    Stmt::Ptr body = stmt();
    if (!body) {
        return nullptr;
    }

    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

auto Parser::return_stmt() -> Stmt::Ptr {
    lexer.next_token(); // consume "return"

    Expr::Ptr return_expr = expr();
    if (!return_expr) {
        return nullptr;
    }

    Token semicolon = lexer.next_token();
    if (semicolon.type != TokenType::Semicolon) {
        make_error("Expect semicolon", semicolon);
        return nullptr;
    }

    return std::make_unique<ReturnStmt>(std::move(return_expr));
}

// expr
auto Parser::expr() -> Expr::Ptr {
    return expr_bp(0);
}

auto Parser::assignable() -> LValue::Ptr {
    Expr::Ptr lvalue = expr_bp(100);

    if (!lvalue) {
        return nullptr;
    }

    return LValue::Ptr(static_cast<LValue *>(lvalue.release()));
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
        case TokenType::String:
            return_expr = std::make_unique<ConstExpr>(ValType::Str, tok.lexeme);
            break;
        case TokenType::Integer:
            return_expr = std::make_unique<ConstExpr>(ValType::Int, tok.lexeme);
            break;
        case TokenType::Floating:
            return_expr = std::make_unique<ConstExpr>(ValType::Flt, tok.lexeme);
            break;
        case TokenType::Identifier:
            return_expr = std::make_unique<LValue>(tok.lexeme);
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