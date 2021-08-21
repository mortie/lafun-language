#include "lafun_parse.h"

#include "parse.h"

using namespace lafun::lafun_ast;

namespace lafun {

#define MAX_TOP_LEVEL_KEYWORD_SIZE 5

void parseLafun(Reader &reader, LafunDocument &document) {
	std::string currentBlock;
	while (true) {
		int ch = reader.peekCh(0);
		if (ch == '\\') {
			if (reader.peekCh(1) == '\\') {
				currentBlock += "\\\\";
				continue;
			}

			std::string possibleKeyword;

			size_t i;
			for (i = 1; i < MAX_TOP_LEVEL_KEYWORD_SIZE; i++) {
				int ch = reader.peekCh(i);
				if (!(ch >= 'a' && ch <= 'z')) {
					break;
				}
				possibleKeyword += ch;
			}

			if (possibleKeyword == "fun" ||
				possibleKeyword == "class" ||
				possibleKeyword == "var") {
				if (!currentBlock.empty()) {
					document.push_back(RawLatex{std::move(currentBlock)});
					currentBlock = "";
				}

				Lexer lexer(reader);
				ast::Declaration decl;
				parseDeclaration(lexer, decl);
				reader = lexer.reader;
				document.push_back(std::move(decl));
			} else {
				// skip it
				for (size_t j = 0; j < i; j++) {
					reader.readCh();
				}
			}
		} else if (ch == '{') {
			// Read till next *matching* }
			currentBlock += reader.readCh();
			size_t numBracesToMatch = 1;
			while (numBracesToMatch > 0) {
				int ch = reader.readCh();
				currentBlock += ch;
				if (ch == EOF) {
					break;
				} else if (ch == '{') {
					numBracesToMatch++;
				} else if (ch == '}') {
					numBracesToMatch--;
				}
			}
		} else if (ch == EOF) {
			if (!currentBlock.empty()) {
				document.push_back(RawLatex{std::move(currentBlock)});
			}
			break;
		} else {
			currentBlock += reader.readCh();
		}
	}
}

}
