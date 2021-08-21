#include "print.h"

#include <cassert>

using namespace lafun::ast;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace lafun {

static void indent(std::ostream &os, int depth) {
	for (int i = 0; i < depth; ++i) {
		os << "    ";
	}
}

static void printExpression(std::ostream &os, Expression &expr, int depth) {
	if (std::holds_alternative<IdentifierExpr>(expr)) {
		os << std::get<IdentifierExpr>(expr).ident;
	} else {
		assert(false);
	}
}

static void printIfStatm(std::ostream &os, IfStatm &statm, int depth) {
	os << "if ";
	printExpression(os, statm.condition, depth);

	os << " {\n";
	printCodeBlock(os, *statm.ifBody, depth + 1);
	indent(os, depth);
	os << "}";

	if (statm.elseBody) {
		os << " else {\n";
		printCodeBlock(os, *statm.elseBody, depth + 1);
		indent(os, depth);
		os << "}";
	}
}

static void printStatement(std::ostream &os, Statement &statm, int depth) {
	if (std::holds_alternative<Expression>(statm)) {
		printExpression(os, std::get<Expression>(statm), depth);
		os << ';';
	} else if (std::holds_alternative<IfStatm>(statm)) {
		printIfStatm(os, std::get<IfStatm>(statm), depth);
	} else {
		assert(false);
	}
}

void printCodeBlock(std::ostream &os, CodeBlock &block, int depth) {
	for (Statement &statm: block.statms) {
		indent(os, depth);
		printStatement(os, statm, depth);
		os << '\n';
	}
}

}
