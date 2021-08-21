#include "print.h"

#include <cassert>

using namespace fun::ast;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace fun {

static void indent(std::ostream &os, int depth) {
	for (int i = 0; i < depth; ++i) {
		os << "    ";
	}
}

static void printExpression(std::ostream &os, const Expression &expr, int depth) {
	std::visit(overloaded {
		[&](const StringLiteralExpr &str) { os << '"' << str.str << '"'; },
		[&](const NumberLiteralExpr &num) { os << num.num; },
		[&](const IdentifierExpr &ident) { os << ident.ident; },
		[&](const BinaryExpr &bin) {
			printExpression(os, *bin.lhs, depth);

			switch (bin.op) {
			case BinaryExpr::ADD: os << " + "; break;
			case BinaryExpr::SUB: os << " - "; break;
			case BinaryExpr::MULT: os << " * "; break;
			case BinaryExpr::DIV: os << " / "; break;
			}

			printExpression(os, *bin.rhs, depth);
		},
		[&](const FuncCallExpr &call) {
			printExpression(os, *call.func, 0);
			os << '(';

			bool first = true;
			for (const std::unique_ptr<Expression> &arg: call.args) {
				if (!first) {
					os << ", ";
				}

				printExpression(os, *arg, depth);
				first = false;
			}

			os << ')';
		},
		[&](const AssignmentExpr &assignment) {
			printExpression(os, *assignment.lhs, depth);
			os << " = ";
			printExpression(os, *assignment.rhs, depth);
		},
		[&](const DeclAssignmentExpr &assignment) {
			printExpression(os, *assignment.lhs, depth);
			os << " := ";
			printExpression(os, *assignment.rhs, depth);
		},
	}, expr);
}

static void printIfStatm(std::ostream &os, const IfStatm &statm, int depth) {
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

void printDeclaration(std::ostream &os, const Declaration &decl, int depth) {
	std::visit(overloaded {
		[&](const ClassDecl &classDecl) {
			os << "\\class{" << classDecl.name << "}{\n";
			printCodeBlock(os, *classDecl.body, depth + 1);
			indent(os, depth);
			os << '}';
		},
		[&](const FuncDecl &funcDecl) {
			os << "\\fun{" << funcDecl.name << "}{";

			bool first = true;
			for (const std::string &arg: funcDecl.args) {
				if (!first) {
					os << ", ";
				}

				os << arg;
				first = false;
			}

			os << "}{\n";
			printCodeBlock(os, *funcDecl.body, depth + 1);
			indent(os, depth);
			os << '}';
		},
		[&](const MethodDecl &methodDecl) {
			os << "\\fun{" << methodDecl.className << "::" << methodDecl.name << "}{";

			bool first = true;
			for (const std::string &arg: methodDecl.args) {
				if (!first) {
					os << ", ";
				}

				os << arg;
				first = false;
			}

			os << "}{\n";
			printCodeBlock(os, *methodDecl.body, depth + 1);
			indent(os, depth);
			os << '}';
		},
	}, decl);
}

static void printStatement(std::ostream &os, const Statement &statm, int depth) {
	std::visit(overloaded {
		[&](const Expression &expr) { printExpression(os, expr, depth); os << ';'; },
		[&](const IfStatm &ifStatm) { printIfStatm(os, ifStatm, depth); },
		[&](const Declaration &decl) { printDeclaration(os, decl, depth); },
	}, statm);
}

void printCodeBlock(std::ostream &os, const CodeBlock &block, int depth) {
	for (const Statement &statm: block.statms) {
		indent(os, depth);
		printStatement(os, statm, depth);
		os << '\n';
	}
}

}
