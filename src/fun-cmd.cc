#include "fun/parse.h"
#include "fun/Lexer.h"
#include "fun/print.h"
#include "fun/IdentResolver.h"
#include "fun/Codegen.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace fun;

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
	std::cout << ss.str();
	std::cout << '\n';

	Lexer lexer(str);

	std::cout << "\n == AST:\n";
	ast::CodeBlock block;
	parseCodeBlock(lexer, block);

	IdentResolver resolver;
	resolver.finalizeBlock(block);

	printCodeBlock(std::cout, block);

	std::cout << "\n == Codegen:\n";
	Codegen codegen;
	for (const auto &statm : block.statms) {
		codegen.add(&statm);
	}
	codegen.generate(std::cout);

	return 0;
}
