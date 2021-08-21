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
using Expression = std::variant<
	StringLiteralExpr,
	NumberLiteralExpr,
	IdentifierExpr,
	BinaryExpr,
	FuncCallExpr>;

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

struct CodeBlock;
struct IfStatm;
using Statement = std::variant<Expression, IfStatm>;

struct IfStatm {
	Expression condition;
	std::unique_ptr<CodeBlock> ifBody;
	std::unique_ptr<CodeBlock> elseBody;
};

struct CodeBlock {
	std::vector<Statement> exprs;
};

struct ClassDecl {
	std::string name;
	CodeBlock body;
};

struct FuncDecl {
	std::string name;
	std::vector<std::string> args;
	CodeBlock body;
};

}
