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

static void parseExpression(Lexer &lexer, Expression &expr);

static void parseParameterList(Lexer &lexer, std::vector<std::unique_ptr<Expression>> &params) {
	lexer.consume(); // '('

	while (true) {
		std::unique_ptr<Expression> param = std::make_unique<Expression>();
		parseExpression(lexer, *param);
		params.push_back(std::move(param));

		Token tok = lexer.peek(0);
		if (tok.kind == TokKind::COMMA) {
			lexer.consume(); // ','
		} else if (tok.kind == TokKind::CLOSE_PAREN) {
			break;
		} else {
			fail(tok, {TokKind::COMMA, TokKind::CLOSE_PAREN});
		}
	}

	lexer.consume(); // ')'
}

static void parseExpression(Lexer &lexer, Expression &expr) {
	TokKind kind = lexer.peek(0).kind;
	if (kind == TokKind::STRING) {
		expr = StringLiteralExpr{std::move(lexer.consume().getStr())};
	} else if (kind == TokKind::NUMBER) {
		expr = NumberLiteralExpr{lexer.consume().getNum()};
	} else if (kind == TokKind::IDENT) {
		expr = IdentifierExpr{std::move(lexer.consume().getStr())};
	} else {
		fail(lexer.peek(0), TokKind::IDENT);
	}

	while (true) {
		kind = lexer.peek(0).kind;
		if (
				kind == TokKind::PLUS || kind == TokKind::MINUS ||
				kind == TokKind::MULT || kind == TokKind::DIV) {
			lexer.consume(); // operator

			BinaryExpr bin;
			bin.lhs = std::make_unique<Expression>(std::move(expr));
			bin.rhs = std::make_unique<Expression>();
			parseExpression(lexer, *bin.rhs);

			if (kind == TokKind::PLUS) {
				bin.op = BinaryExpr::ADD;
			} else if (kind == TokKind::MINUS) {
				bin.op = BinaryExpr::SUB;
			} else if (kind == TokKind::MULT) {
				bin.op = BinaryExpr::MULT;
			} else if (kind == TokKind::DIV) {
				bin.op = BinaryExpr::DIV;
			}

			expr = std::move(bin);
		} else if (kind == TokKind::OPEN_PAREN) {
			FuncCallExpr call;
			call.func = std::make_unique<Expression>(std::move(expr));
			parseParameterList(lexer, call.args);
			expr = std::move(call);
		} else if (kind == TokKind::EQ) {
			lexer.consume(); // '='

			AssignmentExpr assignment;
			assignment.lhs = std::make_unique<Expression>(std::move(expr));
			assignment.rhs = std::make_unique<Expression>();
			parseExpression(lexer, *assignment.rhs);
			expr = std::move(assignment);
		} else if (kind == TokKind::COLONEQ) {
			lexer.consume(); // ':='

			DeclAssignmentExpr assignment;
			assignment.lhs = std::make_unique<Expression>(std::move(expr));
			assignment.rhs = std::make_unique<Expression>();
			parseExpression(lexer, *assignment.rhs);
			expr = std::move(assignment);
		} else {
			break;
		}
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
