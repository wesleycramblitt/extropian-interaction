# ── Interaction unit tests ──

add_executable(test_data_graph         unit/data_graph_test.cpp)
add_executable(test_command            unit/command_test.cpp)
add_executable(test_gestures           unit/gestures_test.cpp)
add_executable(test_interaction_state  unit/interaction_state_test.cpp)
add_executable(test_focus              unit/focus_test.cpp)
add_executable(test_camera_framing     unit/camera_framing_test.cpp)
add_executable(test_binding            unit/binding_test.cpp)

set(TEST_TARGETS
    test_data_graph
    test_command
    test_gestures
    test_interaction_state
    test_focus
    test_camera_framing
    test_binding
)

foreach(target ${TEST_TARGETS})
    target_link_libraries(${target} PRIVATE exd-interaction doctest::doctest_with_main)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${target} COMMAND ${target})
endforeach()
