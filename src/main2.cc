#include "lafun_parse.h"
#include "lafun_print.h"

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
	lafun_ast::LafunDocument document;
	parseLafun(reader, document);
	printLafunDocument(std::cout, document);

	return 0;
}
