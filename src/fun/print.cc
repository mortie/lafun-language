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
	if (std::holds_alternative<StringLiteralExpr>(expr)) {
		os << '"' << std::get<StringLiteralExpr>(expr).str << '"';
	} else if (std::holds_alternative<NumberLiteralExpr>(expr)) {
		os << std::get<NumberLiteralExpr>(expr).num;
	} else if (std::holds_alternative<IdentifierExpr>(expr)) {
		os << std::get<IdentifierExpr>(expr).ident;
	} else if (std::holds_alternative<BinaryExpr>(expr)) {
		const BinaryExpr &bin = std::get<BinaryExpr>(expr);
		printExpression(os, *bin.lhs, depth);

		switch (bin.op) {
		case BinaryExpr::ADD: os << " + "; break;
		case BinaryExpr::SUB: os << " - "; break;
		case BinaryExpr::MULT: os << " * "; break;
		case BinaryExpr::DIV: os << " / "; break;
		}

		printExpression(os, *bin.rhs, depth);
	} else if (std::holds_alternative<FuncCallExpr>(expr)) {
		const FuncCallExpr &call = std::get<FuncCallExpr>(expr);
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
	} else if (std::holds_alternative<AssignmentExpr>(expr)) {
		const AssignmentExpr &assignment = std::get<AssignmentExpr>(expr);

		printExpression(os, *assignment.lhs, depth);
		os << " = ";
		printExpression(os, *assignment.rhs, depth);
	} else if (std::holds_alternative<DeclAssignmentExpr>(expr)) {
		const DeclAssignmentExpr &assignment = std::get<DeclAssignmentExpr>(expr);

		printExpression(os, *assignment.lhs, depth);
		os << " := ";
		printExpression(os, *assignment.rhs, depth);
	} else {
		assert(false);
	}
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
	if (std::holds_alternative<ClassDecl>(decl)) {
		const ClassDecl &classDecl = std::get<ClassDecl>(decl);
		os << "\\class{" << classDecl.name << "}{\n";
		printCodeBlock(os, *classDecl.body, depth + 1);
		indent(os, depth);
		os << '}';
	} else if (std::holds_alternative<FuncDecl>(decl)) {
		const FuncDecl &funcDecl = std::get<FuncDecl>(decl);
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
	} else if (std::holds_alternative<MethodDecl>(decl)) {
		const MethodDecl &methodDecl = std::get<MethodDecl>(decl);
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
	} else {
		assert(false);
	}
}

static void printStatement(std::ostream &os, const Statement &statm, int depth) {
	if (std::holds_alternative<Expression>(statm)) {
		printExpression(os, std::get<Expression>(statm), depth);
		os << ';';
	} else if (std::holds_alternative<IfStatm>(statm)) {
		printIfStatm(os, std::get<IfStatm>(statm), depth);
	} else if (std::holds_alternative<Declaration>(statm)) {
		printDeclaration(os, std::get<Declaration>(statm), depth);
	} else {
		assert(false);
	}
}

void printCodeBlock(std::ostream &os, const CodeBlock &block, int depth) {
	for (const Statement &statm: block.statms) {
		indent(os, depth);
		printStatement(os, statm, depth);
		os << '\n';
	}
}

}
