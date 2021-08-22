#include "prelude.h"

namespace fun {

const char *prelude = R"javascript(
function FUN_Array() {
	return new Array();
}

function FUN_Map() {
	return new Map();
}

function FUN__print() {
	console.log.apply(console, arguments);
}
)javascript";

const std::vector<std::string> preludeNames = {
	"Array", "Map", "print",
};

}
