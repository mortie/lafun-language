#include "parse.h"

#include "fun/parse.h"

using namespace lafun::ast;

namespace lafun {

#define MAX_TOP_LEVEL_KEYWORD_SIZE 5

static size_t findUpwards(LafunDocument &doc, size_t idx, const std::string &name) {
	for (ssize_t i = idx - 1; i >= 0; --i) {
		LafunBlock &block = doc.blocks[i];
		if (std::holds_alternative<FunBlock>(block)) {
			size_t id = fun::resolveUpwardsInDecl(std::get<FunBlock>(block).decl, name);
			if (id != 0) {
				return id;
			}
		}
	}

	for (size_t i = idx + 1; i < doc.blocks.size(); ++i) {
		LafunBlock &block = doc.blocks[i];
		if (std::holds_alternative<FunBlock>(block)) {
			size_t id = fun::resolveDownwardsInDecl(std::get<FunBlock>(block).decl, name);
			if (id != 0) {
				return id;
			}
		}
	}

	return 0;
}

static size_t findDownwards(LafunDocument &doc, size_t idx, const std::string &name) {
	for (size_t i = idx + 1; i < doc.blocks.size(); ++i) {
		LafunBlock &block = doc.blocks[i];
		if (std::holds_alternative<FunBlock>(block)) {
			size_t id = fun::resolveDownwardsInDecl(std::get<FunBlock>(block).decl, name);
			if (id != 0) {
				return id;
			}
		}
	}

	for (ssize_t i = idx - 1; i >= 0; --i) {
		LafunBlock &block = doc.blocks[i];
		if (std::holds_alternative<FunBlock>(block)) {
			size_t id = fun::resolveUpwardsInDecl(std::get<FunBlock>(block).decl, name);
			if (id != 0) {
				return id;
			}
		}
	}

	return 0;
}

void parseLafun(Reader &reader, LafunDocument &document, fun::IdentResolver &resolver) {
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

			if ((possibleKeyword == "fun" ||
				 possibleKeyword == "class" ||
				 possibleKeyword == "var") &&
				reader.peekCh(i) == '{') {

				if (!currentBlock.empty()) {
					document.blocks.emplace_back(RawLatex{std::move(currentBlock)});
					currentBlock = "";
				}

				size_t startIdx = reader.idx;
				fun::Lexer lexer(reader);
				fun::ast::Declaration decl;
				parseDeclaration(lexer, decl);
				reader = lexer.reader_;
				size_t endIdx = reader.idx;
				document.blocks.emplace_back(FunBlock{std::move(decl), fun::ByteRange{startIdx, endIdx}});
			} else {
				// skip it
				for (size_t j = 0; j < i; j++) {
					currentBlock += reader.readCh();
				}
			}
		} else if (ch == '@' || ch == '!') {
			if (!currentBlock.empty()) {
				document.blocks.emplace_back(RawLatex{std::move(currentBlock)});
			}

			reader.readCh();

			// Parse downwards/upwards ref
			std::string ident;

			while (true) {
				int ch = reader.peekCh(0);
				if (!(
						(ch >= 'a' && ch <= 'z') ||
						(ch >= 'A' && ch <= 'Z') ||
						(ch >= '0' && ch <= '9') ||
						ch == '_')) {
					break;
				}
				ident += reader.readCh();
			}

			if (ch == '@') {
				// Upwards ref
				document.blocks.emplace_back(IdentifierUpwardsRef{std::move(ident)});
			} else {
				// Downwards ref
				document.blocks.emplace_back(IdentifierDownwardsRef{std::move(ident)});
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
				document.blocks.emplace_back(RawLatex{std::move(currentBlock)});
			}
			break;
		} else {
			currentBlock += reader.readCh();
		}
	}

	for (LafunBlock &block: document.blocks) {
		if (std::holds_alternative<FunBlock>(block)) {
			resolver.add(&std::get<FunBlock>(block).decl);
		}
	}

	resolver.finalize();

	document.defs = resolver.getDefs();
	document.refs = resolver.getRefs();

	for (size_t i = 0; i < document.blocks.size(); ++i) {
		LafunBlock &block = document.blocks[i];
		if (std::holds_alternative<IdentifierUpwardsRef>(block)) {
			IdentifierUpwardsRef &ref = std::get<IdentifierUpwardsRef>(block);
			ref.id = findUpwards(document, i, ref.ident);
		} else if (std::holds_alternative<IdentifierDownwardsRef>(block)) {
			IdentifierDownwardsRef &ref = std::get<IdentifierDownwardsRef>(block);
			ref.id = findDownwards(document, i, ref.ident);
		}
	}
}

}
