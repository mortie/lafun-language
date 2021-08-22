#include "fun/IdentResolver.h"
#include "fun/prelude.h"

#include "lafun/ast.h"
#include "lafun/parse.h"
#include "lafun/print.h"
#include "fun/IdentResolver.h"
#include "lafun/codegen.h"
#include "lafun/resolve.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace lafun;

int main(int argc, char **argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream stream;
	stream.exceptions(std::ifstream::failbit);
	stream.open(argv[1]);

	std::stringstream ss;
	ss << stream.rdbuf();
	std::string str = ss.str();

	std::cout << " == String:\n";
	std::cout << str;
	std::cout << '\n';

	std::cout << "\n == Document:\n";
	Reader reader(str);
	ast::LafunDocument document;
	fun::IdentResolver resolver;
	for (const std::string &name: fun::preludeNames) {
		resolver.addBuiltin(name);
	}
	parseLafun(reader, document, resolver);

	for (auto &block: document.blocks) {
		if (std::holds_alternative<lafun::ast::FunBlock>(block)) {
			resolver.add(&std::get<lafun::ast::FunBlock>(block).decl);
		}
	}

	resolver.finalize();

	document.defs = resolver.getDefs();
	document.refs = resolver.getRefs();

	// Resolve LaFuN definitions / references
	resolveLafunReferences(document);

	printLafunDocument(std::cout, document);

	std::cout << "\n == Defs:\n";
	for (const auto &def : document.defs) {
		std::cout << def->name << ":" << def->id << " (" << def->range.start << ":" << def->range.end << ")\n";
	}

	std::cout << "\n == Refs:\n";
	for (const auto &ref : document.refs) {
		std::cout << ref->name << ":" << ref->id << " (" << ref->range.start << ":" << ref->range.end << ")\n";
	}

	std::cout << "\n == Codegen:\n";
	codegen(std::cout, str, document);

	return 0;
}
