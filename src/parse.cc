#include "parse.h"

#include "Lexer.h"

namespace lafun {

ast::ClassDecl parseClassDecl(std::string name, std::string body) {
	Lexer lexer(body);
}

}
