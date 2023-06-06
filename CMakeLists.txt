cmake_minimum_required(VERSION 3.24) # this is the version I use, it may well be compatible with lower ones
project(c_facilities)

set(INC_DIR ${PROJECT_SOURCE_DIR}/inc/cfac)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(TST_DIR ${PROJECT_SOURCE_DIR}/tst)
set(DOC_DIR ${PROJECT_SOURCE_DIR}/doc)

set(CMAKE_C_STANDARD 11)

set(WARNINGS 
    -Wall; 
    -Wextra; 
    -pedantic; 
    -Werror;)

set(SANITIZERS 
    -fsanitize=address; 
    -fsanitize=leak; 
    -fsanitize=undefined;
    -fno-omit-frame-pointer;
    )

set(DEBUG_FLAGS -Og; -g; -DDEBUG)
set(RELEASE_FLAGS -O2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(${WARNINGS})

if (DEBUG)
    add_compile_options(${DEBUG_FLAGS})
    add_compile_options(${SANITIZERS})
    add_link_options(${SANITIZERS})
else()
    add_compile_options(${RELEASE_FLAGS})
endif()

find_program(HAS_CPPCHECK NAMES cppcheck)
if (HAS_CPPCHECK)
    set(CMAKE_C_CPPCHECK 
                cppcheck;
                    --enable=all; 
                    --suppress=unusedFunction; # functions go unused in libraries a lot
                    --suppress=uninitvar; # generates some false positives, and anyway compilers do this pretty OK already
                    --suppress=missingIncludeSystem; # cppcheck can't find system headers and I don't care to help it
                    --suppress=unmatchedSuppression;
                    --suppress=redundantAssignment; # generates some false positives in some cases where functions with side-effects are used
                    --suppress=internalAstError; # cppcheck doesn't deal with generics very well, I don't particulary care to hear about it.
    )
endif()


# --- libraries ---

include_directories(${INC_DIR})

add_library(test_utils ${SRC_DIR}/test_utils.c)

add_library(log ${SRC_DIR}/log.c)

add_library(refcount ${SRC_DIR}/refcount.c)

add_library(span ${SRC_DIR}/span.c)
target_link_libraries(span PUBLIC log)

add_library(darray ${SRC_DIR}/darray.c)
target_link_libraries(darray PUBLIC log span)

add_library(list ${SRC_DIR}/list.c)
target_link_libraries(list PUBLIC log)

add_library(hashtable ${SRC_DIR}/hashtable.c)
target_link_libraries(hashtable PUBLIC log darray)


# --- tests ---

if (DEBUG)
    enable_testing()

    function(AddTest TEST_NAME TEST_SOURCE #[[test dependencies...]])
        add_executable(${TEST_NAME} ${TST_DIR}/${TEST_SOURCE})

        target_include_directories(${TEST_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${SRC_DIR})
        target_link_libraries(${TEST_NAME} test_utils ${ARGN})

        add_test(${TEST_NAME} ${TEST_NAME})
    endfunction()

    AddTest(test_utils_test test_utils.test.c)
    AddTest(stat_test stat.test.c)
    AddTest(log_test log.test.c log)
    AddTest(darray_test darray.test.c darray)
    AddTest(span_test span.test.c span)
    AddTest(list_test list.test.c list)
    AddTest(refcount_test refcount.test.c refcount)
    AddTest(hashtable_test hashtable.test.c hashtable)
endif()