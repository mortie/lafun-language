#pragma once

#include <stdexcept>
#include <string>

#include "ast.h"
#include "Lexer.h"

namespace lafun {

struct ParseError: public std::exception {
	ParseError(int line, int column, std::string message):
		line(line), column(column), message(message) {}
	int line;
	int column;
	std::string message;

	const char *what() const noexcept override { return message.c_str(); }
};

void parseCodeBlock(Lexer &lexer, ast::CodeBlock &block);
void parseDeclaration(Lexer &lexer, ast::Declaration &decl);

}
