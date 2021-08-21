OUT ?= build

SRCS := \
	src/Lexer.cc \
	src/main.cc \
	src/parse.cc \
	src/print.cc

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g
LDFLAGS ?=
LDLIBS ?=

ifeq ($(SANITIZE),1)
	CXXFLAGS += -fsanitize=address,undefined
	LDFLAGS += -fsanitize=address,undefined
endif

$(OUT)/lafun: $(patsubst %,$(OUT)/%.o,$(SRCS))
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUT)/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(OUT)
