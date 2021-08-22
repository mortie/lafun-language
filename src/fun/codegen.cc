#include "codegen.h"

namespace fun {

void Codegen::generate(std::ostream &os) {
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

void Codegen::generateStatement(std::ostream &os, const ast::Statement *statm) {
	std::visit(overloaded {
		[&](const ast::Declaration &) {}, // decls are handled in a separate code path
		[&](const auto &statm) { generateStatement(os, &statm); }
	}, *statm);
}

void Codegen::generateStatement(std::ostream &os, const ast::Expression *statm) {
		generateExpressionName(os, generateExpression(os, statm));
		os << ";\n";
	}

void Codegen::generateStatement(std::ostream &os, const ast::IfStatm *statm) {
	// Temporarily swap out the list of names declared in this scope
	std::unordered_set<std::string> outerDeclaredNames(std::move(alreadyDeclared_));

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
	alreadyDeclared_ = std::move(outerDeclaredNames);
}

void Codegen::generateExpressionName(std::ostream &os, ExpressionName name) {
	std::visit(overloaded {
		[&](size_t temp) { os << "temp" << temp; },
		[&](const ast::Identifier *temp) { os << "FUN_" << temp->name; },
		[&](const ast::Expression *temp) {
			std::visit(overloaded {
				[&](const ast::StringLiteralExpr &) { printExpression(os, *temp); },
				[&](const ast::NumberLiteralExpr &) { printExpression(os, *temp); },
				[&](const ast::IdentifierExpr &ident) { os << "FUN_" << ident.ident.name; },
				[&](const auto &) { }, // everything else are not valid ExpressionName
			}, *temp);
		},
	}, name);
}

Codegen::ExpressionName Codegen::generateExpression(std::ostream &os, const ast::Expression *expr) {
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
			if (alreadyDeclared_.find(expr2.ident.name) == alreadyDeclared_.end()) {
				alreadyDeclared_.insert(expr2.ident.name);
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

void Codegen::generateFun(std::ostream &os, const ast::FuncDecl *fun) {
	os << "function FUN_" << fun->ident.name << "(";
	generateParameters(os, fun->args);
	os << ") {\n";
	generateCodeBlock(os, fun->body.get());
	os << "}\n";
}

void Codegen::generateCodeBlock(std::ostream &os, const ast::CodeBlock *block) {
	Codegen codegen;
	for (const auto &statm : block->statms) {
		codegen.add(&statm);
	}
	codegen.generate(os);
}

void Codegen::generateClass(std::ostream &os, const ClassAndMethods &clas) {
	generateClassStart(os, clas.first);
	for (const auto &[_, method] : clas.second) {
		generateClassMethods(os, method);
	}
	generateClassEnd(os);
}

void Codegen::generateClassStart(std::ostream &os, const ast::ClassDecl *clas) {
	os << "class FUN_" << clas->ident.name << " {\n";
	os << "constructor (";
	generateParameters(os, clas->args);
	os << ") {\n";
	generateCodeBlock(os, clas->body.get());
	os << "}\n";
}

void Codegen::generateParameters(std::ostream &os, const std::vector<ast::Identifier> &args) {
	if (!args.empty()) {
		os << args[0].name;
		for (size_t i = 1; i < args.size(); i++) {
			os << ", " << args[i].name;
		}
	}
}

void Codegen::generateClassMethods(std::ostream &os, const ast::MethodDecl *method) {
	os << method->ident.name << "(";
	generateParameters(os, method->args);
	os << ") {\n";
	generateCodeBlock(os, method->body.get());
	os << "}\n";
}

void Codegen::generateClassEnd(std::ostream &os) {
	os << "}\n";
};

}
