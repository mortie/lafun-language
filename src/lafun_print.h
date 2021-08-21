#pragma once

#include <iostream>

#include "lafun_ast.h"

namespace lafun {

void printLafunDocument(std::ostream &os, const lafun_ast::LafunDocument &document);

}
