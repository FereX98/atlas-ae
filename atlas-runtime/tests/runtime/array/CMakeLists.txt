add_executable(array_test array_test.cc)
target_link_libraries(array_test PUBLIC paris pthread GTest::gtest)

add_custom_command(
  TARGET array_test
  POST_BUILD
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM)

add_test(
  NAME array_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND bash ./test.sh $<TARGET_FILE:array_test>)
