#pragma once

#include <string>

namespace lafun {

class Reader {
public:
	Reader(std::string_view string): string_(string) {}

	void reset();
	int readCh();
	int peekCh(size_t n);

	size_t idx = 0;
	int line = 0;
	int column = 0;

private:
	std::string_view string_;
};

}
