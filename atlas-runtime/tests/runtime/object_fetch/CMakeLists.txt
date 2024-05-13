add_executable(object_fetch_test object_fetch_test.cc)
target_link_libraries(object_fetch_test PUBLIC paris pthread GTest::gtest)

add_custom_command(
  TARGET object_fetch_test
  POST_BUILD
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM)

add_test(
  NAME object_fetch_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND bash ./test.sh $<TARGET_FILE:object_fetch_test>)
