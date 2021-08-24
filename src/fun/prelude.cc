#include "prelude.h"

namespace fun {

std::string jsPrelude = R"javascript(/* <Prelude> */
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

	get length() {
		return this.data.length;
	}
}

function FUN_Array() {
	return new FUNclass_Array();
}

class FUNclass_Map {
	constructor() {
		this.data = new Map();
	}

	set(key, val) {
		this.data.set(key, val);
	}

	get(key) {
		return this.data.get(key);
	}

	keys() {
		let arr = FUN_Array();
		for (let key of this.data.keys()) {
			arr.push(key);
		}
		return arr;
	}
}

function FUN_Map() {
	return new FUNclass_Map();
}

function FUN_print() {
	console.log.apply(console, arguments);
}

function FUN_typeof(val) {
	let t = typeof val;
	if (t == "number" || t == "string" || t == "boolean") {
		return t;
	}

	if (t == null) {
		return "none";
	}

	if (t == "object") {
		if (val instanceof FUNclass_Array) {
			return "array";
		} else if (val instanceof FUNclass_Map) {
			return "map";
		}
	}

	return "jsval";
}

let FUN_math = Math;
let FUN_true = true;
let FUN_false = false;
let FUN_none = null;
/* </Prelude> */
)javascript";

std::string jsPostlude = R"javascript(
FUN_main();
)javascript";

const std::vector<std::string> preludeNames = {
	"Array", "Map", "print", "typeof", "math", "true", "false", "none",
};

}
