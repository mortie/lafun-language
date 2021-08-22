#include "prelude.h"

namespace fun {

const char *jsPrelude = R"javascript(
const FUN___builtins = {
	console,
	createArray: function() { return new Array(); },
	createMap: function() { return new Map(); },
};
)javascript";

const char *funPrelude = R"fun(
\fun{print}{str}{
	__builtins.console.log(str);
}

\fun{Array}{}{
	return __builtins.CreateArray();
}

\fun{Map}{}{
	return __builtins.CreateMap();
}
)fun";

}
