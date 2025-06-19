# Project-specific settings
TEST_DIR := source/catch
EMP_DIR := Empirical/include

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++20 -I$(EMP_DIR)/

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

# Compiling different modes
default: default-mode
	@echo Built default version using 'make default-mode'. To use other modes, use the following:
	@echo Efficient mode: make efficient-mode
	@echo Lysis mode: make lysis-mode
	@echo PGG mode: make pgg-mode
	@echo To build the web version use: make web

native: default-mode
web: symbulation.js
all: default-mode efficient-mode lysis-mode pgg-mode symbulation.js

default-mode:	source/native/symbulation_default.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_default.cc -o symbulation_default

efficient-mode:	source/native/symbulation_efficient.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_efficient.cc -o symbulation_efficient

lysis-mode:	source/native/symbulation_lysis.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_lysis.cc -o symbulation_lysis

pgg-mode:	source/native/symbulation_pgg.cc
	$(CXX_nat) $(CFLAGS_nat) source/native/symbulation_pgg.cc -o symbulation_pgg

symbulation.js: source/web/symbulation-web.cc
	$(CXX_web) $(CFLAGS_web) source/web/symbulation-web.cc -o web/symbulation.js

# Debugging
debug:
	@echo Please specify the mode to debug using the following:
	@echo Default mode: make debug-default
	@echo Efficient mode: make debug-efficient
	@echo Lysis mode: make debug-lysis
	@echo PGG mode: make debug-pgg

debug-default: CFLAGS_nat := $(CFLAGS_nat_debug)
debug-default: default-mode 
default-debug: debug-default

debug-efficient: CFLAGS_nat := $(CFLAGS_nat_debug)
debug-efficient: efficient-mode 
efficient-debug: debug-efficient

debug-lysis: CFLAGS_nat := $(CFLAGS_nat_debug)
debug-lysis: lysis-mode 
lysis-debug: debug-lysis

debug-pgg: CFLAGS_nat := $(CFLAGS_nat_debug)
debug-pgg: pgg-mode 
pgg-debug: debug-pgg

debug-web:	CFLAGS_web := $(CFLAGS_web_debug)
debug-web:	symbulation.js
web-debug:	debug-web

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'

# Testing
test:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test ~[integration]
	@echo To run only the tests for each mode, use the following:
	@echo Default mode testing: make test-default
	@echo Efficient mode testing: make test-efficient
	@echo Lysis mode testing: make test-lysis 
	@echo PGG mode testing: make test-pgg

test-debug:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test ~[integration]
	@echo To debug and test for each mode, use the following:
	@echo Default mode: make test-debug-default
	@echo Efficient mode: make test-debug-efficient
	@echo Lysis mode: make test-debug-lysis 
	@echo PGG mode: make test-debug-pgg

test-default:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [default] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-debug-default:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [default] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }


test-efficient:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [efficient] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-debug-efficient:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [efficient] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }


test-lysis:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [lysis] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-debug-lysis:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [lysis] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }


test-pgg:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [pgg] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-debug-pgg:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test [pgg] || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-executable:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test

test-all:
	$(CXX_nat) $(CFLAGS_nat) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-debug-all:
	$(CXX_nat) $(CFLAGS_nat_debug) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test || { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }


# Extras
.PHONY: clean test serve

serve:
	python3 -m http.server

clean:
	rm -f symbulation* web/symbulation.js web/*.js.map web/*.js.map *~ source/*.o

coverage:
	$(CXX_nat) $(CFLAGS_nat_coverage) $(TEST_DIR)/main.cc -o symbulation.test
	./symbulation.test
