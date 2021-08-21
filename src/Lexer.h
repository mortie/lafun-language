#pragma once

#include <string>
#include <cstddef>
#include <variant>
#include <vector>

namespace lafun {

enum class TokKind {
	IDENT,
	NUMBER,
	STRING,

	OPEN_BRACE,
	CLOSE_BRACE,
	OPEN_PAREN,
	CLOSE_PAREN,
	OPEN_BRACKET,
	CLOSE_BRACKET,

	SEMICOLON,
	COMMA,
	EQ, EQEQ, COLONEQ,
	PLUS, PLUSEQ,
	MINUS, MINUSEQ,
	MULT, MULTEQ,
	DIV, DIVEQ,
	COLONCOLON,

	IF, ELSE,

	E_O_F,
	ERROR,
};

struct Token {
	TokKind kind;
	int line;
	int column;

	struct Empty {};
	std::variant<Empty, std::string, double> val;

	std::string &getStr() { return std::get<std::string>(val); }
	double getNum() { return std::get<double>(val); }

	std::string toString();
	static std::string kindToString(TokKind kind);
};

class Lexer {
public:
	Lexer(const std::string &string): string_(string) {}

	Token &peek(size_t n);
	Token consume();

	void reset();

private:
	void skipWhitespace();

	Token readString();
	Token readNumber();
	Token readIdent();

	Token readTok();

	int readCh();
	int peekCh(size_t n);
	Token makeTok(TokKind kind) { return {kind, line_, column_, {}}; }
	Token makeTok(TokKind kind, std::string &&str) { return {kind, line_, column_, std::move(str)}; }
	Token makeTok(TokKind kind, double num) { return {kind, line_, column_, num}; }

	const std::string &string_;
	size_t idx_ = 0;
	int line_ = 0;
	int column_ = 0;

	Token buffer_[4];
	size_t bufidx_ = 0;
};

}
