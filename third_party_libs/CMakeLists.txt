cmake_minimum_required(VERSION 3.12)

add_subdirectory(lz4)
target_include_directories(lz4 INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/lz4/lib>")

set_target_properties(lz4 PROPERTIES FOLDER third_party_libs)
