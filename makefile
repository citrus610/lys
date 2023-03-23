CXX = g++

ifeq ($(PROF), true)
CXXPROF += -pg -no-pie
else
CXXPROF += -s
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -fdiagnostics-color=always -DUNICODE -std=c++20 -Wall -Og -pg -no-pie
else
CXXFLAGS += -DUNICODE -DNDEBUG -std=c++20 -O3 -msse4 -mbmi2 -flto $(CXXPROF) -march=native
endif

ifeq ($(PEXT), true)
CXXFLAGS += -DPEXT
endif

.PHONY: all cli test clean makedir

all: cli test tuner

cli: makedir
	@$(CXX) $(CXXFLAGS) "core\*.cpp" "ai\*.cpp" "cli\*.cpp" $(STATIC_LIB) -o "bin\cli\cli.exe"

test: makedir
	@$(CXX) $(CXXFLAGS) -DTUNER "core\*.cpp" "ai\*.cpp" "test\*.cpp" $(STATIC_LIB) -o "bin\test\test.exe"

tuner: makedir
	@$(CXX) $(CXXFLAGS) -DTUNER "core\*.cpp" "ai\*.cpp" "tuner\*.cpp" $(STATIC_LIB) -o "bin\tuner\tuner.exe"

clean: makedir
	@rm -rf bin
	@make makedir

makedir:
	@mkdir -p bin
	@mkdir -p bin\cli
	@mkdir -p bin\test
	@mkdir -p bin\tuner\data

.DEFAULT_GOAL := cli