function(define_test_suite name dependencies)
    file(GLOB_RECURSE "${name}TestsSources" ${name}/*.cpp)
    message("${name}TestsSources")
    add_executable("${name}Tests" ${${name}TestsSources})
    target_link_libraries("${name}Tests" ${dependencies} gtest_main)
    add_test(NAME "${name}Tests"
            COMMAND "${name}Tests"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
            )
endfunction(define_test_suite)


define_test_suite(concise_json_schema concise_json_schema)
