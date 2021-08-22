#include "lafun/parse.h"
#include "fun/IdentResolver.h"
#include "fun/prelude.h"
#include "fun/Codegen.h"
#include "lafun/print.h"
#include "Reader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

bool streq(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

void usage(const char *argv0) {
	std::cout << "Usage: " << argv0 << " [options] <input file>\n";
	std::cout << "\n";
	std::cout << "Options:\n";
	std::cout << "  --help|-h:          Show this help text\n";
	std::cout << "  --latex <file>:     Write latex to <file>\n";
	std::cout << "  --output|-o <file>: Write generated javascript to <file>\n";
}

int main(int argc, const char **argv) {
	std::ofstream jsFile;
	std::ostream *jsStream = nullptr;

	std::ofstream latexFile;
	std::ostream *latexStream = nullptr;

	std::ifstream inputFile;
	std::istream *inputStream = nullptr;

	bool dashes = false;
	for (int i = 1; i < argc; ++i) {
		const char *opt = argv[i];
		if (!dashes && streq(opt, "--")) {
			dashes = true;
		} else if (!dashes && (streq(opt, "-h") || streq(opt, "--help"))) {
			usage(argv[0]);
			return 0;
		} else if (!dashes && streq(opt, "--latex")) {
			if (i == argc - 1) {
				std::cerr << "Option requires an argument: " << opt << '\n';
				return 1;
			}

			if (streq(argv[i + 1], "-")) {
				latexStream = &std::cout;
			} else {
				latexStream = &latexFile;
				latexFile.open(argv[i + 1]);
				if (!latexFile) {
					std::cerr << "Opening file " << argv[i + 1] << " failed\n";
					return 1;
				}
			}

			i += 1;
		} else if (!dashes && (streq(opt, "--output") || streq(opt, "-o"))) {
			if (i == argc - 1) {
				std::cerr << "Option requires an argument: " << opt << '\n';
				return 1;
			}

			if (streq(argv[i + 1], "-")) {
				jsStream = &std::cout;
			} else {
				jsStream = &jsFile;
				jsFile.open(argv[i + 1]);
				if (!jsFile) {
					std::cerr << "Opening file " << argv[i + 1] << " failed\n";
					return 1;
				}
			}

			i += 1;
		} else if (!dashes && opt[0] == '-' && opt[1] != '\0') {
			std::cerr << "Unknown option: " << opt << '\n';
			usage(argv[0]);
			return 1;
		} else if (!inputStream) {
			if (streq(opt, "-")) {
				inputStream = &std::cin;
			} else {
				inputStream = &inputFile;
				inputFile.open(opt);
				if (!inputFile) {
					std::cerr << "Opening file " << opt << " failed\n";
					return 1;
				}
			}
		} else {
			std::cerr << "Only one input file, please\n";
			usage(argv[0]);
			return 1;
		}
	}

	if (!inputStream) {
		std::cerr << "Missing required input file option\n";
		usage(argv[0]);
		return 1;
	}

	std::stringstream ss;
	ss << inputFile.rdbuf();
	std::string str = ss.str();

	Reader reader{str};
	fun::IdentResolver resolver;
	for (const std::string &name: fun::preludeNames) {
		resolver.addBuiltin(name);
	}

	lafun::ast::LafunDocument document;
	lafun::parseLafun(reader, document, resolver);

	if (jsStream) {
		fun::Codegen gen;
		for (lafun::ast::LafunBlock &block: document.blocks) {
			if (std::holds_alternative<lafun::ast::FunBlock>(block)) {
				std::cerr << "Hello adding fun block\n";
				lafun::ast::FunBlock &funBlock = std::get<lafun::ast::FunBlock>(block);
				gen.add(&funBlock.decl);
			}
		}

		*jsStream << fun::prelude;
		gen.generate(*jsStream);
	}

	if (latexStream) {
		lafun::printLafunDocument(*latexStream, document);
	}

	return 0;
}
