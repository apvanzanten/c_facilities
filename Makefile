BLD_DEBUG_DIR = bld_debug
BLD_RELEASE_DIR = bld_release
BLD_COVERAGE_DIR = bld_coverage

.PHONY: all clean run_tests run_test_coverage

# NOTE lib_coverage is not in here, so a system without support for coverage tooling can still build
all: lib_release lib_debug

lib_release: $(BLD_RELEASE_DIR)/Makefile
	@cd $(BLD_RELEASE_DIR); $(MAKE) --no-print-directory all

lib_debug: $(BLD_DEBUG_DIR)/Makefile
	@cd $(BLD_DEBUG_DIR); $(MAKE) --no-print-directory all

lib_coverage: $(BLD_COVERAGE_DIR)/Makefile
	@cd $(BLD_COVERAGE_DIR); $(MAKE) --no-print-directory all

$(BLD_RELEASE_DIR)/Makefile: CMakeLists.txt
	@cmake -B $(BLD_RELEASE_DIR)

$(BLD_DEBUG_DIR)/Makefile: CMakeLists.txt
	@cmake -D DEBUG=TRUE -B $(BLD_DEBUG_DIR)

$(BLD_COVERAGE_DIR)/Makefile: CMakeLists.txt
	@cmake -D DEBUG=TRUE -D COVERAGE=TRUE -B $(BLD_COVERAGE_DIR)

# NOTE we include 'all' as dependency for test/bench targets, though we only need a subset. We do 
# this to continuously ensure we have a properly working build for 'all' targets.
run_tests: all
	@cd $(BLD_DEBUG_DIR); ctest --output-on-failure -j

# NOTE include run_tests in dependencies; tests should be passing before we run coverage
# NOTE we don't run gcov tool, we assume the user has other tools for that (e.g. vscode extension)
run_test_coverage: all lib_coverage run_tests
	@cd $(BLD_COVERAGE_DIR); ctest -j

run_benches: all
	@cd $(BLD_RELEASE_DIR); ctest --verbose

clean:
	@rm -rf $(BLD_RELEASE_DIR) $(BLD_DEBUG_DIR) $(BLD_COVERAGE_DIR)