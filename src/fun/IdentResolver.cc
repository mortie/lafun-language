#include "IdentResolver.h"

#include <vector>

#include "util.h"

using namespace fun::ast;

namespace fun {

class ScopeStack {
public:
	ScopeStack(IdentResolver &resolver): resolver_(resolver) {
		scopes_.emplace_back();
		scopes_.back()["__builtins"] = BUILTINS;
	}

	void pushScope();
	void popScope();

	void addDef(Identifier &ident);
	void addRedef(Identifier &ident);
	void addRef(Identifier &ident);

	size_t define(const std::string &name);
	size_t redefine(const std::string &name);
	size_t defineTrap(const std::string &name);

	size_t find(const std::string &name);
	size_t tryFind(const std::string &name);

private:
	using Scope = std::unordered_map<std::string, size_t>;
	std::vector<Scope> scopes_;
	IdentResolver &resolver_;

	static constexpr size_t TRAP = ~(size_t)0;
	static constexpr size_t BUILTINS = ~(size_t)1;
};

void ScopeStack::pushScope() {
	scopes_.emplace_back();
}

void ScopeStack::popScope() {
	scopes_.pop_back();
}

void ScopeStack::addDef(Identifier &ident) {
	ident.id = define(ident.name);
	resolver_.addDef(&ident);
}
void ScopeStack::addRedef(Identifier &ident) {
	ident.id = redefine(ident.name);
	resolver_.addDef(&ident);
}
void ScopeStack::addRef(Identifier &ident) {
	ident.id = find(ident.name);
	resolver_.addRef(&ident);
}

size_t ScopeStack::define(const std::string &name) {
	Scope &scope = scopes_.back();
	if (scope.find(name) != scope.end()) {
		throw NameError("Duplicate definition of identifier " + name);
	}

	return scope[name] = resolver_.nextId();
}

size_t ScopeStack::redefine(const std::string &name) {
	Scope &scope = scopes_.back();
	return scope[name] = resolver_.nextId();
}

size_t ScopeStack::defineTrap(const std::string &name) {
	Scope &scope = scopes_.back();
	return scope[name] = TRAP;
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
			size_t id = idIt->second;
			if (id == TRAP) {
				throw NameError("Reference of " + name + " before it's defined");
			}

			return id;
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
			// Disallow temporal dead zones by registering the name
			// in the 'add' step, but as a trap ID, so that it errors if
			// anything tries to reference it
			scope.defineTrap(assignment.ident.name);
			addExpression(scope, *assignment.rhs);
		},
		[&](LookupExpr &lookup) {
			addExpression(scope, *lookup.lhs);
		},
	}, expr);
}

static void finalizeExpression(ScopeStack &scope, Expression &expr) {
	std::visit(overloaded {
		[&](StringLiteralExpr &) {},
		[&](NumberLiteralExpr &) {},
		[&](IdentifierExpr &ident) {
			scope.addRef(ident.ident);
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
			finalizeExpression(scope, *assignment.rhs);
			scope.addRedef(assignment.ident);
		},
		[&](LookupExpr &lookup) {
			finalizeExpression(scope, *lookup.lhs);
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
				scope.addDef(arg);
			}

			finalizeCodeBlock(scope, *classDecl.body);
			scope.popScope();
		},
		[&](FuncDecl &funcDecl) {
			scope.pushScope();
			for (Identifier &arg: funcDecl.args) {
				scope.addDef(arg);
			}

			finalizeCodeBlock(scope, *funcDecl.body);
			scope.popScope();
		},
		[&](MethodDecl &methodDecl) {
			scope.addRef(methodDecl.classIdent);

			scope.pushScope();
			for (Identifier &arg: methodDecl.args) {
				scope.addDef(arg);
			}

			finalizeCodeBlock(scope, *methodDecl.body);
			scope.popScope();
		},
	}, decl);
}

static void addDeclaration(ScopeStack &scope, Declaration &decl) {
	std::visit(
		[&](auto &decl) {
			scope.addDef(decl.ident);
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

void IdentResolver::finalizeBlock(CodeBlock &block) {
	ScopeStack scope(*this);
	finalizeCodeBlock(scope, block);
}

static void resolveInDecl(const Declaration &decl, const std::string &name, std::vector<size_t> &ids);
static void resolveInCodeBlock(const CodeBlock &block, const std::string &name, std::vector<size_t> &ids);

static void resolveInExpression(const Expression &expr, const std::string &name, std::vector<size_t> &ids) {
	std::visit(overloaded {
		[&](const StringLiteralExpr &) {},
		[&](const NumberLiteralExpr &) {},
		[&](const IdentifierExpr &ident) {
			if (ident.ident.name == name) {
				ids.push_back(ident.ident.id);
			}
		},
		[&](const BinaryExpr &bin) {
			resolveInExpression(*bin.lhs, name, ids);
			resolveInExpression(*bin.rhs, name, ids);
		},
		[&](const FuncCallExpr &call) {
			resolveInExpression(*call.func, name, ids);
			for (const std::unique_ptr<Expression> &arg: call.args) {
				resolveInExpression(*arg, name, ids);
			}
		},
		[&](const AssignmentExpr &assignment) {
			resolveInExpression(*assignment.lhs, name, ids);
			resolveInExpression(*assignment.rhs, name, ids);
		},
		[&](const DeclAssignmentExpr &assignment) {
			if (assignment.ident.name == name) {
				ids.push_back(assignment.ident.id);
			}
			resolveInExpression(*assignment.rhs, name, ids);
		},
		[&](const LookupExpr &lookup) {
			resolveInExpression(*lookup.lhs, name, ids);
		},
	}, expr);
}

static void resolveInStatement(const Statement &statm, const std::string &name, std::vector<size_t> &ids) {
	std::visit(overloaded {
		[&](const Expression &expr) {
			resolveInExpression(expr, name, ids);
		},
		[&](const IfStatm &ifStatm) {
			resolveInExpression(ifStatm.condition, name, ids);
			resolveInCodeBlock(*ifStatm.ifBody, name, ids);
			if (ifStatm.elseBody) {
				resolveInCodeBlock(*ifStatm.elseBody, name, ids);
			}
		},
		[&](const Declaration &decl) {
			resolveInDecl(decl, name, ids);
		}
	}, statm);
}

static void resolveInCodeBlock(const CodeBlock &block, const std::string &name, std::vector<size_t> &ids) {
	for (const Statement &statm: block.statms) {
		resolveInStatement(statm, name, ids);
	}
}

static void resolveInDecl(const Declaration &decl, const std::string &name, std::vector<size_t> &ids) {
	auto resolveInArgs = [&](const std::vector<Identifier> &args) {
		for (const Identifier &ident: args) {
			if (ident.name == name) {
				ids.push_back(ident.id);
			}
		}
	};

	std::visit(overloaded {
		[&](const ClassDecl &classDecl) {
			if (classDecl.ident.name == name) {
				ids.push_back(classDecl.ident.id);
			}

			resolveInArgs(classDecl.args);
			resolveInCodeBlock(*classDecl.body, name, ids);
		},
		[&](const FuncDecl &funcDecl) {
			if (funcDecl.ident.name == name) {
				ids.push_back(funcDecl.ident.id);
			}

			resolveInArgs(funcDecl.args);
			resolveInCodeBlock(*funcDecl.body, name, ids);
		},
		[&](const MethodDecl &methodDecl) {
			if (methodDecl.ident.name == name) {
				ids.push_back(methodDecl.ident.id);
			}

			resolveInArgs(methodDecl.args);
			resolveInCodeBlock(*methodDecl.body, name, ids);
		},
	}, decl);
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
