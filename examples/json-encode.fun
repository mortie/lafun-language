\section{Introduction}

JSON (JavaScript Object Notation) is a standard data interchange format.
It supports the following basic data types:

\begin{itemize}
\item Number: A real number. It's represented in JSON as a decimal number,
	with an optional fractional part.
\item String: A sequence of Unicode code points. It's represented in JSON by
	an opening quote $"$, followed by a sequence of characters, followed by
	a closing quote $"$. Special values, such as quotes, can be escaped with a backslash.
\item Boolean: A boolean value. Represented by the text $true$ or $false$.
\item Array: A sequence of JSON values. Represented by an opening bracket $[$,
	followed by a sequence of JSON values separated by a comma $,$, followed by
	a closing bracket $]$.
\item Object: A map of strings to JSON values. Representing by an opening brace $\{$,
	followed by a sequence of key/value pairs separated by a comma $,$ followed by
	a closing brace $\}$. A key/value pair is representedy by a string, followed by
	a colon $:$, followed by a JSON value.
\end{itemize}

\section{Implementation}

The @jsonEncode function implements a basic encoding function.
It acts as the entry-point into JSON encoding, and its only responsibility is to delegate
to other functions based on the type of the argument.
It takes an argument @val, which is the value to encode.

\fun{jsonEncode}{val}{
	t := typeof(val);
	if t == "number" {
		return jsonEncodeNumber(val);
	} else if t == "string" {
		return jsonEncodeString(val);
	} else if t == "boolean" {
		return jsonEncodeBoolean(val);
	} else if t == "none" {
		return jsonEncodeNone(val);
	} else if t == "array" {
		return jsonEncodeArray(val);
	} else if t == "map" {
		return jsonEncodeObject(val);
	} else {
		return none;
	}
}

The @jsonEncodeNumber function has to produce a string which contains the
decimal expansion of a real number.

\fun{jsonEncodeNumber}{num}{
	return num.toString();
}

The @jsonEncodeString function has to produce a string which contains
a JSON string literal, with proper escaping of quotes and backslashes.

\fun{jsonEncodeString}{str}{
	string := '"';
	index := 0;
	while index < str.length {
		ch := str.at(index);
		if ch == '\\' {
			string = string + '\\\\';
		} else if ch == '\"' {
			string = string + '\\"';
		} else {
			string = string + ch;
		}

		index = index + 1;
	}

	string = string + "\"";
	return string;
}

The @jsonEncodeBoolean function needs to produce a string which contains either
$true$ or $false$.

\fun{jsonEncodeBoolean}{bool}{
	if bool {
		return "true";
	} else {
		return "false";
	}
}

The @jsonEncodeNone function needs to produce a string which contains $null$.

\fun{jsonEncodeNone}{val}{
	return "null";
}

The @jsonEncodeArray function needs to produce an array of JSON values,
which is represented by an opening bracket $[$, followed by comma-separated
JSON values, followed by a closing bracket $]$.
It uses the @jsonEncode function to do all the heavy lifting of actually encoding
the values.

\fun{jsonEncodeArray}{arr}{
	string := "[";
	index := 0;
	while index < arr.length {
		if index != 0 {
			string = string + ", ";
		}

		string = string + jsonEncode(arr.get(index));
		index = index + 1;
	}

	string = string + "]";
	return string;
}

The @jsonEncodeObject function needs to produce a map of string keys to JSON values,
which is represented by an opening brace $\{$, followed by comma-separated
key/value pairs, followed by a closing brace $\}$.
It uses the @jsonEncode function to encode the values, and the @jsonEncodeString function
to encode the keys.
It also uses the @indent helper function to produce proper indentation.

\fun{jsonEncodeObject}{map}{
	string := "{";
	keys := map.keys();
	index := 0;
	while index < keys.length {
		if index != 0 {
			string = string + ", ";
		}

		key := keys.get(index);
		val := map.get(key);
		string = string + jsonEncodeString(key);
		string = string + ": ";
		string = string + jsonEncode(val);

		index = index + 1;
	}

	string = string + "}";
	return string;
}

\section{Demonstration}

Here's a simple example program which uses @jsonEncode to encode a fairly complicated
nested JSON structure.

\fun{main}{}{
	topLevel := Map();
	topLevel.set("exampleString", "I'm a JSON string");
	topLevel.set("someNumber", 100.57);
	topLevel.set("nothing", none);

	exampleArray := Array();
	exampleArray.push(100);

	nestedMap := Map();
	nestedMap.set("yes", true);
	nestedMap.set("nope", false);
	nestedMap.set("how are you?", "\"Fine\", thanks");
	exampleArray.push(nestedMap);

	topLevel.set("exampleArray", exampleArray);
	print(jsonEncode(topLevel));
}

The output of that code should be:
\begin{verbatim}
{"exampleString": "I'm a JSON string", "someNumber": 100.57, "nothing": null,
"exampleArray": [100, {"yes": true, "nope": false, "how are you?": "\"Fine\", thanks"}]}
\end{verbatim}
