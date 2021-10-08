# Project-specific settings
TEST_MAIN := source/catch/main
EMP_DIR := ../Empirical/include

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/

# Native compiler information
CXX_nat := g++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g -DEMP_TRACK_MEM $(CFLAGS_all)
CFLAGS_nat_coverage := --coverage $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web_all := -s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'stringToUTF8', 'UTF8ToString']" -s TOTAL_MEMORY=268435456 --js-library $(EMP_DIR)/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback', '_empDoCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1 #--embed-file configs
OFLAGS_web := -Oz -DNDEBUG
OFLAGS_web_debug := -g4 -Oz -pedantic -Wno-dollar-in-identifier-extension

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) $(OFLAGS_web_all)
CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) $(OFLAGS_web_all)


default: source/native/symbulation.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation.cc -o symbulation
	@echo To build the web version use: make web

native: symbulation
web: symbulation.js
all: default_mode efficient_mode lysis_mode pgg_mode symbulation.js

debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	symbulation

debug-web:	CFLAGS_web := $(CFLAGS_web_debug)
debug-web:	symbulation.js

web-debug:	debug-web

default_mode:	source/native/symbulation_default.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_default.cc -o symbulation_default

efficient_mode:	source/native/symbulation_efficient.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_efficient.cc -o symbulation_efficient

lysis_mode:	source/native/symbulation_lysis.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_lysis.cc -o symbulation_lysis

pgg_mode:	source/native/symbulation_pgg.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_pgg.cc -o symbulation_pgg

symbulation.js: source/web/symbulation-web.cc
	$(CXX_web) $(CFLAGS_web) source/web/symbulation-web.cc -o web/symbulation.js

.PHONY: clean test serve

serve:
	python3 -m http.server

clean:
	rm -f symbulation web/symbulation.js web/*.js.map web/*.js.map *~ source/*.o

test:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_MAIN).cc -o symbulation.test
	./symbulation.test

test-debug:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_MAIN).cc -o symbulation.test
	./symbulation.test

coverage:
	$(CXX_nat) $(CFLAGS_nat_coverage) $(TEST_MAIN).cc -o symbulation.test
	./symbulation.test


# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
