OUT ?= build

LIBSRCS := \
	src/fun/Codegen.cc \
	src/fun/IdentResolver.cc \
	src/fun/Lexer.cc \
	src/fun/parse.cc \
	src/fun/prelude.cc \
	src/fun/print.cc \
	src/lafun/parse.cc \
	src/lafun/print.cc \
	src/lafun/codegen.cc \
	src/Reader.cc \
#

MAINSRCS := \
	src/fun-cmd.cc \
	src/lafun-cmd.cc \
#

ALLSRCS := ${MAINSRCS} ${LIBSRCS}

CXX ?= g++
CXXFLAGS ?= -Isrc -std=c++17 -Wall -Wextra -g
LDFLAGS ?=
LDLIBS ?=

ifeq ($(SANITIZE),1)
	CXXFLAGS += -fsanitize=address,undefined
	LDFLAGS += -fsanitize=address,undefined
endif

all: $(OUT)/fun $(OUT)/lafun

$(OUT)/fun: $(OUT)/src/fun-cmd.cc.o $(patsubst %,$(OUT)/%.o,$(LIBSRCS))
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUT)/lafun: $(OUT)/src/lafun-cmd.cc.o $(patsubst %,$(OUT)/%.o,$(LIBSRCS))
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUT)/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OUT)/%.cc.d: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MM -MT "$(patsubst %,$(OUT)/%.o,$<) $(patsubst %,$(OUT)/%.d,$<)" -o $@ $<

include $(patsubst %,$(OUT)/%.d,$(ALLSRCS))

.PHONY: clean
clean:
	rm -rf $(OUT)
