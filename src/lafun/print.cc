#include "print.h"

#include <cassert>
#include <variant>

#include "util.h"
#include "fun/print.h"

using namespace lafun::ast;

namespace lafun {

void printLafunDocument(std::ostream &os, const LafunDocument &document) {
	for (const auto &block : document) {
		std::visit(overloaded {
			[&](const RawLatex &block) { os << block.str; },
			[&](const IdentifierUpwardsRef &ref) { os << '@' << ref.ident; },
			[&](const IdentifierDownwardsRef &ref) { os << '!' << ref.ident; },
			[&](const fun::ast::Declaration &decl) { fun::printDeclaration(os, decl); },
		}, block);
	}
}

}
