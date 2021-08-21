#include "parse.h"

#include <utility>

using namespace lafun::ast;

namespace lafun {

static void fail(Token &tok, TokKind kind) {
	std::string message = "Expected ";
	message += Token::kindToString(kind);
	message += ", got ";
	message += Token::kindToString(tok.kind);
	throw ParseError(tok.line, tok.column, std::move(message));
}

static void fail(Token &tok, std::initializer_list<TokKind> kinds) {
	std::string message = "Expected ";
	bool first = true;
	for (const TokKind &kind: kinds) {
		if (!first) {
			message += " or ";
		}

		message += Token::kindToString(kind);
		first = false;
	}

	message += ", got ";
	message += Token::kindToString(tok.kind);
	throw ParseError(tok.line, tok.column, std::move(message));
}

static void expect(Lexer &lexer, TokKind kind) {
	Token &tok = lexer.peek(0);
	if (tok.kind != kind) {
		fail(tok, kind);
	}
}

static void parseExpression(Lexer &lexer, Expression &expr) {
	Token &tok = lexer.peek(0);
	TokKind kind = tok.kind;
	if (kind == TokKind::IDENT) {
		expr = IdentifierExpr{std::move(std::get<std::string>(lexer.consume().val))};
	} else {
		fail(tok, TokKind::IDENT);
	}
}

static void parseIfStatm(Lexer &lexer, IfStatm &statm) {
	lexer.consume(); // 'if'

	// condition
	statm.ifBody = std::make_unique<CodeBlock>();
	parseExpression(lexer, statm.condition);

	// '{'
	expect(lexer, TokKind::OPEN_BRACE);
	lexer.consume(); // '{'

	// <code block>
	parseCodeBlock(lexer, *statm.ifBody);

	// '}'
	expect(lexer, TokKind::CLOSE_BRACE);
	lexer.consume(); // '}'

	// optional: 'else'
	if (lexer.peek(0).kind == TokKind::ELSE) {
		lexer.consume(); // 'if'
		statm.elseBody = std::make_unique<CodeBlock>();

		// either: '{'
		if (lexer.peek(0).kind == TokKind::OPEN_BRACE) {
			lexer.consume(); // '{'

			// <code block>
			parseCodeBlock(lexer, *statm.elseBody);

			// '}'
			expect(lexer, TokKind::CLOSE_BRACE);
			lexer.consume(); // '}'
		}

		// or: 'if'
		else if (lexer.peek(0).kind == TokKind::IF) {
			// <if statement>
			statm.elseBody->statms.push_back(IfStatm{});
			parseIfStatm(lexer, std::get<IfStatm>(statm.elseBody->statms.back()));

			// Tail-recurse here, don't look for a '}'
			return;
		}

		// or: error
		else {
			fail(lexer.peek(0), {TokKind::OPEN_BRACE, TokKind::IF});
		}
	}
}

static void parseStatement(Lexer &lexer, Statement &statm) {
	TokKind kind = lexer.peek(0).kind;
	if (kind == TokKind::IF) {
		statm.emplace<IfStatm>();
		parseIfStatm(lexer, std::get<IfStatm>(statm));
	} else {
		statm.emplace<Expression>();
		parseExpression(lexer, std::get<Expression>(statm));
		expect(lexer, TokKind::SEMICOLON);
		lexer.consume(); // ';'
	}
}

void parseCodeBlock(Lexer &lexer, CodeBlock &block) {
	while (true) {
		TokKind kind = lexer.peek(0).kind;
		if (kind == TokKind::E_O_F || kind == TokKind::CLOSE_BRACE) {
			break;
		}

		block.statms.emplace_back();
		parseStatement(lexer, block.statms.back());
	}
};

}
