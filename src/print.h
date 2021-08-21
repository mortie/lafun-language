#pragma once

#include <iostream>

#include "ast.h"

namespace lafun {

void printCodeBlock(std::ostream &os, ast::CodeBlock &block, int depth = 0);

}
