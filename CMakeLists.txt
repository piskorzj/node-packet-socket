project(packet-socket)
cmake_minimum_required(VERSION 2.8)

add_definitions( -Wall -Wextra -Werror)

function(add_module module_name)
  set(MOCKS)
  set(REDUCED_MODULES ${MODULES})
  list(REMOVE_ITEM REDUCED_MODULES ${module_name})
  foreach(MODULE ${REDUCED_MODULES})
    list(APPEND MOCKS ${MOCKS_DIR}/${MODULE}.cc)
  endforeach(MODULE)
  add_executable(test_${module_name}
    ${SRC_DIR}/${module_name}.cc
    ${TESTS_DIR}/${module_name}.cc
    ${MOCKS})
  target_link_libraries(test_${module_name} CppUTest CppUTestExt)
  add_test(test_of_${module_name} test_${module_name} -v -c)
endfunction(add_module)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cc_tests)
set(MOCKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cc_mocks)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULES
  socket
)

enable_testing()

#LOOP OVER MODULES
set(TEST_TARGETS)
foreach(MODULE ${MODULES})
  list(APPEND TEST_TARGETS test_${MODULE})
  add_module(${MODULE})
endforeach(MODULE)

add_custom_target(build_node_gyp COMMAND node-gyp build
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(run_wrapper_test
  COMMAND node -e "process.exitCode = require('./build/Release/test_wrapper.node').Run('', '-v', '-c')"
  DEPENDS build_node_gyp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)

add_custom_target(run_poller_test
  COMMAND node -e "process.exitCode = require('./build/Release/test_poller.node').Run('', '-v', '-c')"
  DEPENDS build_node_gyp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)

add_custom_target(run_all_node_tests
DEPENDS run_wrapper_test run_poller_test)

add_test(test_of_node_cpp_parts make run_all_node_tests)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -VV)
