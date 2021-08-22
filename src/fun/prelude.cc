#include "prelude.h"

namespace fun {

std::string prelude = R"javascript(// <Prelude>
function FUN_Array() {
	return new Array();
}

function FUN_Map() {
	return new Map();
}

function FUN__print() {
	console.log.apply(console, arguments);
}
// </Prelude>
)javascript";

const std::vector<std::string> preludeNames = {
	"Array", "Map", "print",
};

}
