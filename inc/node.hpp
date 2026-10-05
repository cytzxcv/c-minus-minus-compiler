#pragma once

#include "token.hpp"

#include <memory>
#include <string>
// #include <variant>
#include <vector>

class Declaration {
public:
    virtual ~Declaration() = default;
};

class Statement {
public:
    virtual ~Statement() = default;
};

class Expression {
public:
    virtual ~Expression() = default;
};

class FunctionDeclaration : public Declaration {
    std::string name;
    std::vector<std::unique_ptr<Statement>> stmts;

    FunctionDeclaration(std::string);
};

class VariableDeclaration : public Declaration, public Statement {
    Token type; // int, float
    std::string name;
    std::unique_ptr<Expression> rhs;
};

class ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expr;

    ExpressionStatement(std::unique_ptr<Expression>);
};

class BlockStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> block;

    BlockStatement();
};

/*
class ForStatement : public Statement {
    std::unique_ptr<std::variant<VariableDeclaration, Expression>>
}
*/