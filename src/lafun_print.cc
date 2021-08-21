#include "lafun_print.h"

#include <cassert>
#include <variant>

#include "print.h"

using namespace lafun::lafun_ast;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace lafun {

void printLafunDocument(std::ostream &os, const LafunDocument &document) {
	for (const auto &block : document) {
		std::visit(overloaded {
				[&](const RawLatex &block) { os << block.str; },
				[&](const IdentifierRef &ref) { os << '@' << ref.identifier; },
				[&](const ast::CodeBlock &block) { printCodeBlock(os, block); },
			}, block);
	}
}

}
