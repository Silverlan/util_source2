cmake_minimum_required(VERSION 3.12)

add_subdirectory(lz4-cmake)
set_target_properties(lz4 PROPERTIES FOLDER third_party_libs)
