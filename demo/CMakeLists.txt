# Extropian Interaction Demo — CLI smoke test

cmake_minimum_required(VERSION 3.21)
project(extropian-interaction-demo LANGUAGES CXX)

add_executable(extropian-interaction-demo main.cpp)

target_link_libraries(extropian-interaction-demo PRIVATE exd-interaction)
target_include_directories(extropian-interaction-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
