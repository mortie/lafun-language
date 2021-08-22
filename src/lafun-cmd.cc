#include "lafun/parse.h"
#include "lafun/print.h"

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
	parseLafun(reader, document);
	printLafunDocument(std::cout, document);

	std::cout << "\n == Defs:\n";
	for (const auto &def : document.defs) {
		std::cout << def->name << ":" << def->id << " (" << def->range.start << ":" << def->range.end << ")\n";
	}

	std::cout << "\n == Refs:\n";
	for (const auto &ref : document.refs) {
		std::cout << ref->name << ":" << ref->id << " (" << ref->range.start << ":" << ref->range.end << ")\n";
	}

	return 0;
}
