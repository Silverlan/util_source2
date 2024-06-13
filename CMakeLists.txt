include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

option(UTIL_SOURCE2_STATIC "Build as static library?" ON)

if(${UTIL_SOURCE2_STATIC})
	set(LIB_TYPE STATIC)
else()
	set(LIB_TYPE SHARED)
endif()

set(PROJ_NAME util_source2)
pr_add_library(${PROJ_NAME} ${LIB_TYPE})

pr_add_dependency(${PROJ_NAME} vfilesystem TARGET PUBLIC)
pr_add_external_dependency(${PROJ_NAME} lz4 LIBRARY)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

if(NOT ${UTIL_SOURCE2_STATIC})
	pr_add_compile_definitions(${PROJ_NAME} -DUS2_SHARED -DUS2_EXPORT -DSHUTIL_STATIC -DMUTIL_STATIC -DVFILESYSTEM_STATIC)
endif()

add_subdirectory("third_party_libs")
add_dependencies(${PROJ_NAME} lz4)

pr_finalize(${PROJ_NAME})
