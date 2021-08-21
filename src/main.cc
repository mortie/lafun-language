#include "parse.h"
#include "Lexer.h"
#include "parse.h"
#include "print.h"

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
	std::cout << ss.str();
	std::cout << '\n';

	Lexer lexer(str);
	std::cout << " == Tokens:\n";
	while (true) {
		Token tok = lexer.consume();
		std::cout << tok.toString() << '\n';
		if (tok.kind == TokKind::E_O_F) {
			break;
		}
	}

	lexer.reset();

	std::cout << "\n == AST:\n";
	ast::CodeBlock block;
	parseCodeBlock(lexer, block);
	printCodeBlock(std::cout, block);

	return 0;
}
