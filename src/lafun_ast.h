#pragma once

#include <variant>
#include <vector>

#include "ast.h"

namespace lafun::lafun_ast {

struct RawLatex {
	std::string str;
};

struct IdentifierRef {
	std::string identifier;
};

using LafunBlock = std::variant<
	ast::CodeBlock,
	RawLatex,
	IdentifierRef>;

using LafunDocument = std::vector<LafunBlock>;

}
