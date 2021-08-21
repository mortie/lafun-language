#include "parse.h"
#include "Lexer.h"

#include <fstream>
#include <iostream>

using namespace lafun;

int main(int argc, char **argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream stream(argv[1]);
	std::string str;
	while (stream.good() && !stream.eof()) {
		char buf[1024];
		std::streamsize n = stream.readsome(buf, sizeof(buf));
		if (n == 0) break;
		str.append(buf, n);
	}

	Lexer lexer(str);
	while (true) {
		Token tok = lexer.readTok();
		std::cout << tok.toString() << '\n';
		if (tok.tok == Tok::E_O_F) {
			break;
		}
	}

	return 0;
}
