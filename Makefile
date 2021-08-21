OUT ?= build

SRCS := \
	src/main.cc \
	src/parse.cc \
	src/Lexer.cc

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?=

$(OUT)/lafun: $(patsubst %,$(OUT)/%.o,$(SRCS))
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUT)/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(OUT)
