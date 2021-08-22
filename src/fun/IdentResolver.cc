#include "IdentResolver.h"

#include <vector>

#include "util.h"

using namespace fun::ast;

namespace fun {

class ScopeStack {
public:
	ScopeStack(IdentResolver &resolver): resolver_(resolver) {}

	void pushScope();
	void popScope();

	size_t define(const std::string &name);
	size_t tryDefine(const std::string &name);

	size_t find(const std::string &name);
	size_t tryFind(const std::string &name);

private:
	using Scope = std::unordered_map<std::string, size_t>;
	std::vector<Scope> scopes_;
	IdentResolver &resolver_;
};

void ScopeStack::pushScope() {
	scopes_.emplace_back();
}

void ScopeStack::popScope() {
	scopes_.pop_back();
}

size_t ScopeStack::define(const std::string &name) {
	Scope &scope = scopes_.back();
	if (scope.find(name) != scope.end()) {
		throw NameError("Duplicate definition of identifier " + name);
	}

	return scope[name] = resolver_.nextId();
}

size_t ScopeStack::tryDefine(const std::string &name) {
	Scope &scope = scopes_.back();
	auto it = scope.find(name);
	if (it == scope.end()) {
		return scope[name] = resolver_.nextId();
	}

	return it->second;
}

size_t ScopeStack::find(const std::string &name) {
	size_t id = tryFind(name);
	if (id == 0) {
		throw NameError("Undefined identifier " + name);
	}

	return id;
}

size_t ScopeStack::tryFind(const std::string &name) {
	for (auto it = scopes_.rbegin(); it != scopes_.rend(); --it) {
		Scope &scope = *it;
		auto idIt = scope.find(name);
		if (idIt != scope.end()) {
			return idIt->second;
		}
	}

	return resolver_.tryFind(name);
}

void IdentResolver::add(Declaration *decl) {
	std::visit(overloaded {
		[&](ClassDecl &classDecl) {
			if (decls_.find(classDecl.ident.name) != decls_.end()) {
				throw NameError("Duplicate definition of " + classDecl.ident.name);
			}

			classDecl.ident.id = nextId();
			ids_[classDecl.ident.name] = classDecl.ident.id;
			decls_[classDecl.ident.name] = decl;
		},
		[&](FuncDecl &funcDecl) {
			if (decls_.find(funcDecl.ident.name) != decls_.end()) {
				throw NameError("Duplicate definition of " + funcDecl.ident.name);
			}

			funcDecl.ident.id = nextId();
			ids_[funcDecl.ident.name] = funcDecl.ident.id;
			decls_[funcDecl.ident.name] = decl;
		},
		[&](MethodDecl &methodDecl) {
			std::string name = methodDecl.classIdent.name + "::" + methodDecl.ident.name;
			if (decls_.find(name) != decls_.end()) {
				throw NameError("Duplicate definition of " + name);
			}

			methodDecl.ident.id = nextId();
			ids_[name] = methodDecl.ident.id;
			decls_[name] = decl;
		},
	}, *decl);
}

static void finalizeCodeBlock(ScopeStack &scope, CodeBlock &block);

static void addDeclaration(ScopeStack &scope, Declaration &decl) {
	std::visit(overloaded {
		[&](ClassDecl &classDecl) { scope.define(classDecl.ident.name); },
		[&](FuncDecl &funcDecl) { scope.define(funcDecl.ident.name); },
		[&](MethodDecl &methodDecl) { scope.define(methodDecl.ident.name); },
	}, decl);
}

static void addExpression(ScopeStack &scope, Expression &expr) {
	std::visit(overloaded {
		[&](StringLiteralExpr &) {},
		[&](NumberLiteralExpr &) {},
		[&](IdentifierExpr &) {},
		[&](BinaryExpr &bin) {
			addExpression(scope, *bin.lhs);
			addExpression(scope, *bin.rhs);
		},
		[&](FuncCallExpr &call) { addExpression(scope, *call.func); },
		[&](AssignmentExpr &assignment) {
			addExpression(scope, *assignment.lhs);
			addExpression(scope, *assignment.rhs);
		},
		[&](DeclAssignmentExpr &assignment) {
			scope.tryDefine(assignment.ident.name);
			addExpression(scope, *assignment.rhs);
		},
	}, expr);
}

static void finalizeExpression(ScopeStack &scope, Expression &expr) {
	std::visit(overloaded {
		[&](StringLiteralExpr &) {},
		[&](NumberLiteralExpr &) {},
		[&](IdentifierExpr &ident) { ident.ident.id = scope.find(ident.ident.name); },
		[&](BinaryExpr &bin) {
			finalizeExpression(scope, *bin.lhs);
			finalizeExpression(scope, *bin.rhs);
		},
		[&](FuncCallExpr &call) { addExpression(scope, *call.func); },
		[&](AssignmentExpr &assignment) {
			finalizeExpression(scope, *assignment.lhs);
			finalizeExpression(scope, *assignment.rhs);
		},
		[&](DeclAssignmentExpr &assignment) {
			assignment.ident.id = scope.find(assignment.ident.name);
			finalizeExpression(scope, *assignment.rhs);
		},
	}, expr);
}

static void addStatement(ScopeStack &scope, Statement &statm) {
	std::visit(overloaded {
		[&](Expression &expr) { addExpression(scope, expr); },
		[&](IfStatm &) {},
		[&](Declaration &decl) { addDeclaration(scope, decl); }
	}, statm);
}

static void finalizeStatement(ScopeStack &scope, Statement &statm) {
	std::visit(overloaded {
		[&](Expression &expr) { finalizeExpression(scope, expr); },
		[&](IfStatm &ifStatm) {
			scope.pushScope();
			addExpression(scope, ifStatm.condition);
			finalizeExpression(scope, ifStatm.condition);

			finalizeCodeBlock(scope, *ifStatm.ifBody);
			if (ifStatm.elseBody) {
				finalizeCodeBlock(scope, *ifStatm.elseBody);
			}

			scope.popScope();
		},
		[&](Declaration &decl) { addDeclaration(scope, decl); }
	}, statm);
}

static void finalizeCodeBlock(ScopeStack &scope, CodeBlock &block) {
	scope.pushScope();

	for (Statement &statm: block.statms) {
		addStatement(scope, statm);
	}

	for (Statement &statm: block.statms) {
		finalizeStatement(scope, statm);
	}

	scope.popScope();
}

static void finalizeDeclaration(ScopeStack &scope, Declaration &decl) {
	std::visit(overloaded {
		[&](ClassDecl &classDecl) {
			finalizeCodeBlock(scope, *classDecl.body);
		},
		[&](FuncDecl &funcDecl) {
			scope.pushScope();
			for (Identifier &arg: funcDecl.args) {
				arg.id = scope.define(arg.name);
			}

			finalizeCodeBlock(scope, *funcDecl.body);
			scope.popScope();
		},
		[&](MethodDecl &methodDecl) {
			size_t classId = scope.find(methodDecl.classIdent.name);
			methodDecl.classIdent.id = classId;

			scope.pushScope();
			for (Identifier &arg: methodDecl.args) {
				arg.id = scope.define(arg.name);
			}

			finalizeCodeBlock(scope, *methodDecl.body);
			scope.popScope();
		},
	}, decl);
}

void IdentResolver::finalize() {
	ScopeStack stack(*this);
	for (auto &[_, decl]: decls_) {
		finalizeDeclaration(stack, *decl);
	}
}

size_t IdentResolver::tryFind(const std::string &name) {
	auto it = ids_.find(name);
	if (it == ids_.end()) {
		return 0;
	}

	return it->second;
}

}
