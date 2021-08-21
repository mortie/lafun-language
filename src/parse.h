#include "ast.h"

namespace lafun {

ast::ClassDecl parseClassDecl(std::string name, std::string body);

ast::FuncDecl parseFuncDecl(std::string name, std::string args, std::string body);

}
