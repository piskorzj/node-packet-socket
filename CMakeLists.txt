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

set(MODULES
  
)

enable_testing()


set(TEST_TARGETS)
foreach(MODULE ${MODULES})
  list(APPEND TEST_TARGETS test_${MODULE})
endforeach(MODULE)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -VV
  DEPENDS ${TEST_TARGETS})

