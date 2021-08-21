#pragma once

#include <variant>
#include <vector>

#include "fun/ast.h"

namespace lafun::ast {

struct RawLatex {
	std::string str;
};

struct IdentifierRef {
	std::string identifier;
};

using LafunBlock = std::variant<
	fun::ast::Declaration,
	RawLatex,
	IdentifierRef>;

using LafunDocument = std::vector<LafunBlock>;

}
