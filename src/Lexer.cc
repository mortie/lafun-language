#include "Lexer.h"

#include <sstream>

namespace lafun {

// TODO: Bring this out to a shared utility function?
template<typename... Args>
std::string cat(Args &&...args) {
	std::stringstream stream;
	((stream << args), ...);
	return stream.str();
}

static char parseDigit(char ch, int radix) {
	int num;
	if (ch >= '0' && ch <= '9') {
		num = ch - '0';
	} else if (ch >= 'a' && ch <= 'z') {
		num = ch - 'a' + 10;
	} else if (ch >= 'A' && ch <= 'Z') {
		num = ch - 'A' + 10;
	} else {
		return -1;
	}

	if (num < radix) {
		return num;
	}

	return -1;
}

static double parseDigitString(const std::string &digits, int radix) {
	double num = 0;
	double mult = 1;
	for (ssize_t idx = digits.size() - 1; idx >= 0; --idx) {
		num += digits[idx] * mult;
		mult *= radix;
	}

	return num;
}

Token Lexer::readTok() {
	skipWhitespace();

	int ch = peekCh(0);
	switch (ch) {
	case '{': readCh(); return makeTok(Tok::OPEN_BRACE);
	case '}': readCh(); return makeTok(Tok::CLOSE_BRACE);
	case '(': readCh(); return makeTok(Tok::OPEN_PAREN);
	case ')': readCh(); return makeTok(Tok::CLOSE_PAREN);
	case '[': readCh(); return makeTok(Tok::OPEN_BRACKET);
	case ']': readCh(); return makeTok(Tok::CLOSE_BRACKET);
	case EOF: readCh(); return makeTok(Tok::E_O_F);
	}

	int ch2 = peekCh(1);
	if (ch == '=' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::EQEQ);
	} else if (ch == ':' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::COLONEQ);
	} else if (ch == '=') {
		readCh();
		return makeTok(Tok::EQ);
	} else if (ch == '+' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::PLUSEQ);
	} else if (ch == '+') {
		readCh();
		return makeTok(Tok::PLUS);
	} else if (ch == '-' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::PLUSEQ);
	} else if (ch == '-') {
		readCh();
		return makeTok(Tok::PLUS);
	} else if (ch == '*' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::MULTEQ);
	} else if (ch == '*') {
		readCh(); readCh();
		return makeTok(Tok::MULT);
	} else if (ch == '/' && ch2 == '=') {
		readCh(); readCh();
		return makeTok(Tok::DIVEQ);
	} else if (ch == '/') {
		readCh();
		return makeTok(Tok::DIV);
	} else if (ch == ':' && ch2 == ':') {
		readCh(); readCh();
		return makeTok(Tok::COLONCOLON);
	}

	if (ch == '"' || ch == '\'') {
		return readString();
	} else if (ch >= 0 && ch <= 9) {
		return readNumber();
	} else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
		Token ident = readIdent();
		if (ident.tok != Tok::IDENT) {
			return ident;
		}

		std::string str = std::get<std::string>(ident.val);
		if (str == "if") {
			return makeTok(Tok::IF);
		} else if (str == "else") {
			return makeTok(Tok::ELSE);
		}

		return ident;
	}

	return makeTok(Tok::ERROR, cat("Unexpected character: '", (char)ch, '\''));
}

void Lexer::skipWhitespace() {
	while (true) {
		int ch = peekCh(0);
		if (ch != ' ' && ch != '\t' && ch != '\n') {
			break;
		}

		readCh();
	}
}

Token Lexer::readString() {
	std::string str;
	char terminator = readCh();

	while (true) {
		int ch = readCh();
		if (ch == EOF) {
			return makeTok(Tok::ERROR, "Unexpected EOF");
		}

		if (ch == '\\') {
			ch = readCh();
			if (ch == EOF) {
				return makeTok(Tok::ERROR, "Unexpected EOF");
			}

			if (ch == 'n') {
				str += '\n';
			} else if (ch == 't') {
				str += '\t';
			} else if (ch == 'r') {
				str += '\r';
			} else if (ch == 'e') {
				str += 0x1b; // Escape character
			} else if (ch == '\'' || ch == '"' || ch == '\\') {
				str += ch;
			} else {
				return makeTok(Tok::ERROR,
						cat("Unexpected escaped character '", (char)ch, '\''));
			}
		} else if (ch == terminator) {
			return makeTok(Tok::STRING, std::move(str));
		} else {
			str += ch;
		}
	}
}

Token Lexer::readNumber() {
	int radix = 10;
	if (peekCh(0) == 0 && peekCh(1) == 'x') {
		radix = 16;
		readCh(); readCh();
	} else if (peekCh(0) == 0 && peekCh(1) == 'b') {
		radix = 2;
		readCh(); readCh();
	} else if (peekCh(0) == 0 && peekCh(1) == 'o') {
		radix = 8;
		readCh(); readCh();
	}

	if (parseDigit(peekCh(0), radix) < 0) {
		return makeTok(Tok::ERROR, "Invalid number");
	}

	char digit;
	std::string digits = "";

	double integral;
	while ((digit = parseDigit(peekCh(0), radix)) >= 0) {
		digits += digit;
		readCh();
	}
	integral = parseDigitString(digits, radix);

	double fractional = 0;
	if (peekCh(0) == '.' && parseDigit(peekCh(1), radix) >= 0) {
		readCh();
		digits = "";
		while ((digit = parseDigit(peekCh(0), radix)) >= 0) {
			digits += digit;
			readCh();
		}
		fractional = parseDigitString(digits, radix);

		double div = 1;
		for (size_t i = 0; i < digits.size(); ++i) {
			div *= radix;
		}

		fractional /= div;
	}

	return makeTok(Tok::NUMBER, integral + fractional);
}

Token Lexer::readIdent() {
	std::string str;
	str += readCh();

	while (true) {
		int ch = peekCh(0);
		if (!(
				(ch >= 'a' && ch <= 'z') ||
				(ch >= 'A' && ch <= 'Z') ||
				(ch >= '0' && ch <= '9') ||
				ch == '_')) {
			break;
		}

		str += ch;
		readCh();
	}

	return makeTok(Tok::IDENT, std::move(str));
}

int Lexer::readCh() {
	if (idx_ >= string_.size()) {
		return EOF;
	}

	char ch = string_[idx_++];
	column_ += 1;
	if (ch == '\n') {
		column_ = 0;
		line_ += 1;
	}

	return ch;
}

int Lexer::peekCh(int n) {
	if (idx_ + n >= string_.size()) {
		return EOF;
	}

	return string_[idx_ + n];
}

}
