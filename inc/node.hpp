#pragma once

#include "token.hpp"

#include <memory>
#include <string>
#include <variant>
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

class Assignable : public Expression {
    std::string name;
    std::vector<std::unique_ptr<Expression>> dim;

    Assignable(std::string);
};

class FunctionDeclaration : public Declaration {
    Token type; // int, float
    std::string name;
    std::vector<std::unique_ptr<Assignable>> parem;
    std::vector<std::unique_ptr<Statement>> stmts;

    FunctionDeclaration(Token, std::string);
};

class VariableDeclaration : public Declaration, public Statement {
    Token type; // int, float
    std::string name;
    std::unique_ptr<Expression> rhs;

    VariableDeclaration(Token, std::string, std::unique_ptr<Expression>);
};

class ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expr;

    ExpressionStatement(std::unique_ptr<Expression>);
};

class BlockStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> block;

    BlockStatement();
};

class IfStatement : public Statement {
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Statement> if_branch, else_branch;

    IfStatement(std::unique_ptr<Expression>, std::unique_ptr<Statement>, std::unique_ptr<Statement>);
};

class ForStatement : public Statement {
    std::unique_ptr<std::variant<VariableDeclaration, Expression>> init;
    std::unique_ptr<Expression> cond, iter;
    std::unique_ptr<Statement> stmt;

    ForStatement(std::unique_ptr<std::variant<VariableDeclaration, Expression>>, std::unique_ptr<Expression>,
                 std::unique_ptr<Expression>, std::unique_ptr<Statement>);
};

class WhileStatement : public Statement {
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Statement> stmt;

    WhileStatement(std::unique_ptr<Expression>, std::unique_ptr<Statement>);
};

class ConstantExpression : public Expression {
    Token type; // int, float
    std::string value;

    ConstantExpression(Token, std::string);
};

class BinaryExpression : public Expression {
    Token op;
    std::unique_ptr<Expression> lhs, rhs;
};

class UnaryExpression : public Expression {
    Token op;
    std::unique_ptr<Expression> rhs;
};
