#pragma once

#include <iostream>

#include "ast.h"

namespace lafun {

void printCodeBlock(std::ostream &os, const ast::CodeBlock &block, int depth = 0);

}
