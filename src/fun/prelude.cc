#include "prelude.h"

namespace fun {

std::string prelude = R"javascript(/* <Prelude> */
class FUNclass_Array {
	constructor() {
		this.data = [];
	}

	push(val) {
		return this.data.push(val);
	}

	pop() {
		return this.data.pop()
	}

	get(idx) {
		return this.data[idx];
	}

	set(idx, val) {
		return this.data[idx] = val;
	}
}

function FUN_Array() {
	return new FUNclass_Array();
}

function FUN_print() {
	console.log.apply(console, arguments);
}
/* </Prelude> */
)javascript";

const std::vector<std::string> preludeNames = {
	"Array", "print",
};

}
