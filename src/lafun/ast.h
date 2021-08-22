#pragma once

#include <variant>
#include <vector>

#include "fun/ast.h"

namespace lafun::ast {

struct RawLatex {
	std::string str;
};

struct IdentifierUpwardsRef {
	std::string ident;
	size_t id = 0;
};

struct IdentifierDownwardsRef {
	std::string ident;
	size_t id = 0;
};

using LafunBlock = std::variant<
	fun::ast::Declaration,
	RawLatex,
	IdentifierUpwardsRef,
	IdentifierDownwardsRef>;

using LafunDocument = std::vector<LafunBlock>;

}
