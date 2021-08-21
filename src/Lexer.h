#include <string>
#include <cstddef>
#include <variant>

namespace lafun {

enum class Tok {
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
	Tok tok;
	int line;
	int column;

	struct Empty {};
	std::variant<Empty, std::string, double> val;

	std::string toString();
};

class Lexer {
public:
	Lexer(const std::string &string): string_(string) {}

	Token readTok();

private:
	void skipWhitespace();

	Token readString();
	Token readNumber();
	Token readIdent();

	int readCh();
	int peekCh(int n);
	Token makeTok(Tok tok) { return Token{tok, line_, column_, {}}; }
	Token makeTok(Tok tok, std::string &&str) { return Token{tok, line_, column_, std::move(str)}; }
	Token makeTok(Tok tok, double num) { return Token{tok, line_, column_, num}; }

	const std::string &string_;
	size_t idx_ = 0;
	int line_ = 0;
	int column_ = 0;
};

}
