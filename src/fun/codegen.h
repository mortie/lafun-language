#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <variant>
#include <sstream>

#include "util.h"
#include "ast.h"
#include "print.h"

namespace fun {

class Codegen {
	using Methods = std::unordered_map<std::string, const ast::MethodDecl *>;
	using ClassAndMethods = std::pair<const ast::ClassDecl *, std::unordered_map<std::string, const ast::MethodDecl *>>;
	std::unordered_map<std::string, ClassAndMethods> classes_;
	std::vector<const ast::FuncDecl *> funs_;
	std::vector<const ast::Statement *> statms_; // except decls

	using TemporaryId = size_t;
	using ExpressionName = std::variant<TemporaryId, const ast::Identifier *, const ast::Expression *>;
	// The name of "x := 5" is the subexpression "x"
	// The name of "foo.bar := 5" is the subexpression "foo.bar" (when we support . operator)
	// The name of "10 + (x := 5)" is some unique size_t not given to any other temporary

public:
	void add(const ast::Statement *statm) {
		if (auto decl = std::get_if<ast::Declaration>(statm)) {
			add(decl);
		} else {
			statms_.push_back(statm);
		}
	}

	void add(const ast::Declaration *decl) {
		std::visit([&](const auto &decl) { add(&decl); }, *decl);
	}

	void add(const ast::ClassDecl *decl) {
		classes_[decl->ident.name].first = decl;
	}

	void add(const ast::MethodDecl *decl) {
		classes_[decl->classIdent.name].second[decl->ident.name] = decl;
	}

	void add(const ast::FuncDecl *decl) {
		funs_.push_back(decl);
	}

	void generate(std::ostream &os) {
		for (const auto &[_, classAndMethods] : classes_) {
			generateClass(os, classAndMethods);
		}

		for (const auto &fun : funs_) {
			generateFun(os, fun);
		}

		for (const auto &statm : statms_) {
			generateStatement(os, statm);
		}
	}



private:
	std::unordered_set<std::string> alreadyDeclared;

	void generateStatement(std::ostream &os, const ast::Statement *statm) {
		std::visit(overloaded {
			[&](const ast::Declaration &) {}, // decls are handled in a separate code path
			[&](const auto &statm) { generateStatement(os, &statm); }
			}, *statm);
	}

	void generateStatement(std::ostream &os, const ast::Expression *statm) {
		generateExpressionName(os, generateExpression(os, statm));
		os << ";\n";
	}

	void generateStatement(std::ostream &os, const ast::IfStatm *statm) {
		// Temporarily swap out the list of names declared in this scope
		std::unordered_set<std::string> outerDeclaredNames(std::move(alreadyDeclared));

		os << "{\n";
		auto name = generateExpression(os, &statm->condition);
		os << "if (";
		generateExpressionName(os, name);
		os << ") {\n";
		generateCodeBlock(os, statm->ifBody.get());
		os << "}\n";
		if (statm->elseBody) {
			os << "else {\n";
			generateCodeBlock(os, statm->elseBody.get());
			os << "}\n";
		}
		os << "}\n";

		// Move them back
		alreadyDeclared = std::move(outerDeclaredNames);
	}

	size_t counter = 0;
	size_t count() {
		return counter++;
	}

	void generateExpressionName(std::ostream &os, ExpressionName name) {
		std::visit(overloaded {
				[&](size_t temp) { os << "temp" << temp; },
				[&](const ast::Identifier *temp) { os << "var_" << temp->name; },
				[&](const ast::Expression *temp) {
					std::visit(overloaded {
							[&](const ast::StringLiteralExpr &) { printExpression(os, *temp); },
							[&](const ast::NumberLiteralExpr &) { printExpression(os, *temp); },
							[&](const ast::IdentifierExpr &ident) { os << "var_" << ident.ident.name; },
							[&](const auto &) { }, // everything else are not valid ExpressionName
							}, *temp);
				},
				}, name);
	}

	ExpressionName generateExpression(std::ostream &os, const ast::Expression *expr) {
		return std::visit(overloaded {
				[&](const ast::StringLiteralExpr &) -> ExpressionName { return expr; },
				[&](const ast::NumberLiteralExpr &) -> ExpressionName { return expr; },
				[&](const ast::IdentifierExpr &) -> ExpressionName { return expr; },
				[&](const ast::BinaryExpr &expr2) -> ExpressionName {
					// Recursively generate the two operands to get the expression names lhs and rhs
					// Emit temp = lhs + rhs
					// Return temp as the expression name
					auto lhsName = generateExpression(os, expr2.lhs.get());
					auto rhsName = generateExpression(os, expr2.rhs.get());
					auto temp = count();
					os << "const temp" << temp << " = ";
					generateExpressionName(os, lhsName);
					switch (expr2.op) {
						case ast::BinaryExpr::ADD: os << " + "; break;
						case ast::BinaryExpr::SUB: os << " - "; break;
						case ast::BinaryExpr::MULT: os << " * "; break;
						case ast::BinaryExpr::DIV: os << " / "; break;
					}
					generateExpressionName(os, rhsName);
					os << ";\n";
					return temp;
				},
				[&](const ast::FuncCallExpr &expr2) -> ExpressionName {
					// Recursively generate the function and arguments
					// Emit temp = fun(args...)
					// Return temp as the expression name
					auto funName = generateExpression(os, expr2.func.get());
					std::vector<ExpressionName> argNames;
					for (const auto &arg : expr2.args) {
						argNames.emplace_back(generateExpression(os, arg.get()));
					}
					auto temp = count();
					os << "const temp" << temp << " = ";
					generateExpressionName(os, funName);
					os << "(";
					for (const auto &argName : argNames) {
						generateExpressionName(os, argName);
					}
					os << ");\n";
					return temp;
				},
				[&](const ast::AssignmentExpr &expr2) -> ExpressionName {
					// Recursively generate the rhs
					// Emit lhs = rhsName
					// Return lhs as the expression name
					auto rhsName = generateExpression(os, expr2.rhs.get());
					generateExpressionName(os, expr2.lhs.get());
					os << " = ";
					generateExpressionName(os, rhsName);
					os << ";\n";
					return expr2.lhs.get();
				},
				[&](const ast::DeclAssignmentExpr &expr2) -> ExpressionName {
					// Recursively generate the rhs
					// Emit declaration if not already declared
					// Emit lhs = rhsName
					// Return lhs as the expression name
					auto rhsName = generateExpression(os, expr2.rhs.get());
					if (alreadyDeclared.find(expr2.ident.name) == alreadyDeclared.end()) {
						alreadyDeclared.insert(expr2.ident.name);
						os << "let ";
						generateExpressionName(os, &expr2.ident);
						os << ";\n";
					}
					generateExpressionName(os, &expr2.ident);
					os << " = ";
					generateExpressionName(os, rhsName);
					os << ";\n";
					return &expr2.ident;
				},
				}, *expr);
	}

	void generateFun(std::ostream &os, const ast::FuncDecl *fun) {
		os << "function " << fun->ident.name << "(";
		generateParameters(os, fun->args);
		os << ") {\n";
		generateCodeBlock(os, fun->body.get());
		os << "}\n";
	}

	void generateCodeBlock(std::ostream &os, const ast::CodeBlock *block) {
		Codegen codegen;
		for (const auto &statm : block->statms) {
			codegen.add(&statm);
		}
		codegen.generate(os);
	}

	void generateClass(std::ostream &os, const ClassAndMethods &clas) {
		generateClassStart(os, clas.first);
		for (const auto &[_, method] : clas.second) {
			generateClassMethods(os, method);
		}
		generateClassEnd(os);
	}

	void generateClassStart(std::ostream &os, const ast::ClassDecl *clas) {
		os << "class " << clas->ident.name << " {\n";
		os << "constructor (";
		generateParameters(os, clas->args);
		os << ") {\n";
		generateCodeBlock(os, clas->body.get());
		os << "}\n";
	}

	void generateParameters(std::ostream &os, const std::vector<ast::Identifier> &args) {
		if (!args.empty()) {
			os << args[0].name;
			for (size_t i = 1; i < args.size(); i++) {
				os << ", " << args[i].name;
			}
		}
	}

	void generateClassMethods(std::ostream &os, const ast::MethodDecl *method) {
		os << method->ident.name << "(";
		generateParameters(os, method->args);
		os << ") {\n";
		generateCodeBlock(os, method->body.get());
		os << "}\n";
	}

	void generateClassEnd(std::ostream &os) {
		os << "}\n";
	};

};

}
