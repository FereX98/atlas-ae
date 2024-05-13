add_executable(runtime_raw_test runtime_raw_test.cc)
target_link_libraries(runtime_raw_test PUBLIC paris pthread GTest::gtest)

add_custom_command(
  TARGET runtime_raw_test
  POST_BUILD
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM)

add_test(
  NAME runtime_raw_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND bash ./test.sh $<TARGET_FILE:runtime_raw_test>)
