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
	for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
		Scope &scope = *it;
		auto idIt = scope.find(name);
		if (idIt != scope.end()) {
			return idIt->second;
		}
	}

	return 0;
}

void IdentResolver::add(Declaration *decl) {
	decls_.push_back(decl);
}

static void finalizeCodeBlock(ScopeStack &scope, CodeBlock &block);
static void addDeclaration(ScopeStack &scope, Declaration &decl);
static void finalizeDeclaration(ScopeStack &scope, Declaration &decl);

static void addExpression(ScopeStack &scope, Expression &expr) {
	std::visit(overloaded {
		[&](StringLiteralExpr &) {},
		[&](NumberLiteralExpr &) {},
		[&](IdentifierExpr &) {},
		[&](BinaryExpr &bin) {
			addExpression(scope, *bin.lhs);
			addExpression(scope, *bin.rhs);
		},
		[&](FuncCallExpr &call) {
			addExpression(scope, *call.func);
			for (std::unique_ptr<Expression> &arg: call.args) {
				addExpression(scope, *arg);
			}
		},
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
		[&](IdentifierExpr &ident) {
			ident.ident.id = scope.find(ident.ident.name);
		},
		[&](BinaryExpr &bin) {
			finalizeExpression(scope, *bin.lhs);
			finalizeExpression(scope, *bin.rhs);
		},
		[&](FuncCallExpr &call) {
			finalizeExpression(scope, *call.func);
			for (std::unique_ptr<Expression> &arg: call.args) {
				finalizeExpression(scope, *arg);
			}
		},
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
		[&](Declaration &decl) { finalizeDeclaration(scope, decl); }
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
			scope.pushScope();
			for (Identifier &arg: classDecl.args) {
				arg.id = scope.define(arg.name);
			}

			finalizeCodeBlock(scope, *classDecl.body);
			scope.popScope();
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
			methodDecl.classIdent.id = scope.find(methodDecl.classIdent.name);

			scope.pushScope();
			for (Identifier &arg: methodDecl.args) {
				arg.id = scope.define(arg.name);
			}

			finalizeCodeBlock(scope, *methodDecl.body);
			scope.popScope();
		},
	}, decl);
}

static void addDeclaration(ScopeStack &scope, Declaration &decl) {
	std::visit(overloaded {
		[&](ClassDecl &classDecl) {
			classDecl.ident.id = scope.define(classDecl.ident.name);
		},
		[&](FuncDecl &funcDecl) {
			funcDecl.ident.id = scope.define(funcDecl.ident.name);
		},
		[&](MethodDecl &methodDecl) {
			methodDecl.ident.id = scope.define(methodDecl.ident.name);
		},
	}, decl);
}

void IdentResolver::finalize() {
	ScopeStack scope(*this);
	scope.pushScope();

	for (auto decl: decls_) {
		addDeclaration(scope, *decl);
	}

	for (auto decl: decls_) {
		finalizeDeclaration(scope, *decl);
	}

	scope.popScope();
}

void IdentResolver::resolveCodeBlock(CodeBlock &block) {
	ScopeStack scope(*this);
	finalizeCodeBlock(scope, block);
}

void resolveInDecl(Declaration &decl, const std::string &name, std::vector<size_t> &ids) {
}

size_t resolveUpwardsInDecl(Declaration &decl, const std::string &name) {
	std::vector<size_t> ids;
	resolveInDecl(decl, name, ids);
	if (ids.size() > 0) {
		return ids.back();
	}

	return 0;
}

size_t resolveDownwardsInDecl(Declaration &decl, const std::string &name) {
	std::vector<size_t> ids;
	resolveInDecl(decl, name, ids);
	if (ids.size() > 0) {
		return ids.front();
	}

	return 0;
}

}
