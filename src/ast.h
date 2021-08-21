#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace lafun::ast {

struct StringLiteralExpr;
struct NumberLiteralExpr;
struct IdentifierExpr;
struct BinaryExpr;
struct FuncCallExpr;
struct AssignmentExpr;
struct DeclAssignmentExpr;
using Expression = std::variant<
	StringLiteralExpr,
	NumberLiteralExpr,
	IdentifierExpr,
	BinaryExpr,
	FuncCallExpr,
	AssignmentExpr,
	DeclAssignmentExpr>;

struct StringLiteralExpr {
	std::string str;
};

struct NumberLiteralExpr {
	double num;
};

struct IdentifierExpr {
	std::string ident;
};

struct BinaryExpr {
	enum Oper {ADD, SUB, MULT, DIV};

	Oper op;
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
};

struct FuncCallExpr {
	std::unique_ptr<Expression> func;
	std::vector<std::unique_ptr<Expression>> args;
};

struct AssignmentExpr {
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
};

struct DeclAssignmentExpr {
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
};

struct ClassDecl;
struct FuncDecl;
struct MethodDecl;
using Declaration = std::variant<ClassDecl, FuncDecl, MethodDecl>;

struct CodeBlock;
struct IfStatm;
using Statement = std::variant<
	Expression,
	IfStatm,
	Declaration>;

struct IfStatm {
	Expression condition;
	std::unique_ptr<CodeBlock> ifBody;
	std::unique_ptr<CodeBlock> elseBody;
};

struct ClassDecl {
	std::string name;
	std::unique_ptr<CodeBlock> body;
};

struct FuncDecl {
	std::string name;
	std::vector<std::string> args;
	std::unique_ptr<CodeBlock> body;
};

struct MethodDecl {
	std::string className;
	std::string name;
	std::vector<std::string> args;
	std::unique_ptr<CodeBlock> body;
};

struct CodeBlock {
	std::vector<Statement> statms;
};

}
