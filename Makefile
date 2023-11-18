BLD_DEBUG_DIR = bld_debug
BLD_RELEASE_DIR = bld_release

.PHONY: all clean run_tests

all: lib_release lib_debug

lib_release: $(BLD_RELEASE_DIR)/Makefile
	@cd $(BLD_RELEASE_DIR); $(MAKE) --no-print-directory all

lib_debug: $(BLD_DEBUG_DIR)/Makefile
	@cd $(BLD_DEBUG_DIR); $(MAKE) --no-print-directory all

$(BLD_RELEASE_DIR)/Makefile: CMakeLists.txt
	@cmake -B $(BLD_RELEASE_DIR)

$(BLD_DEBUG_DIR)/Makefile: CMakeLists.txt
	@cmake -D DEBUG=TRUE -B $(BLD_DEBUG_DIR)

# NOTE we include 'all' as dependency for run_tests, though we only need a subset. We do this to 
# continuously ensure we have a properly working build for 'all' targets
run_tests: all
	@cd $(BLD_DEBUG_DIR); ctest --output-on-failure

run_benches: all
	@cd $(BLD_RELEASE_DIR); ctest --verbose


clean:
	@rm -rf $(BLD_RELEASE_DIR) $(BLD_DEBUG_DIR)