#pragma once

#include <unordered_map>
#include <cstddef>

#include "ast.h"

namespace fun {

struct NameError: public std::exception {
	NameError(std::string message): message(message) {}

	std::string message;

	const char *what() const noexcept override {
		return message.c_str();
	}
};

class IdentResolver {
public:
	void add(ast::Declaration *decl);
	void finalize();
	size_t nextId() { return id_++; }

	void resolveCodeBlock(ast::CodeBlock &block);

private:
	std::vector<ast::Declaration *> decls_;
	size_t id_ = 1;
};

}
